require("dotenv").config();
const express = require("express");
const axios = require("axios");
const nodemailer = require("nodemailer");

const app = express();
app.use(express.json());
// We need this to parse Twilio's incoming webhooks
app.use(express.urlencoded({ extended: true }));

const transporter = nodemailer.createTransport({
  service: "gmail",
  auth: { user: process.env.EMAIL_USER, pass: process.env.EMAIL_PASS }
});

const ACCOUNT_SID = process.env.TWILIO_SID;
const AUTH_TOKEN = process.env.TWILIO_AUTH;

// 🔥 This acts as the memory switch between Twilio and the ESP32
let locationRequested = false;

// ------------------------------------------------
// 1. TWILIO INBOUND WEBHOOK (When YOU send a WhatsApp)
// ------------------------------------------------
app.post("/webhook", (req, res) => {
    const incomingMessage = req.body.Body;
    const senderNumber = req.body.From;

    console.log(`\n📩 Incoming WhatsApp from ${senderNumber}: "${incomingMessage}"`);
    
    // Flip the switch so the ESP32 knows to send data
    locationRequested = true;
    
    // Tell Twilio we received it
    res.status(200).send("Command Received");
});

// ------------------------------------------------
// 2. ESP32 POLLING ROUTE
// ------------------------------------------------
app.get("/check", (req, res) => {
    if (locationRequested) {
        res.send("YES");
    } else {
        res.send("NO");
    }
});

// ------------------------------------------------
// 3. ESP32 DATA DISPATCH ROUTE (Sends final alerts)
// ------------------------------------------------
app.get("/track", async (req, res) => {
  const lat = req.query.lat || "0";
  const lon = req.query.lon || "0";
  const spd = req.query.spd || "0";

  console.log(`📍 Interrogation Response -> Lat: ${lat}, Lon: ${lon}, Speed: ${spd}`);

  // 1. Instantly reply to the ESP32 so it doesn't time out!
  res.status(200).send("✅ Data received, processing alerts in background.");

  // Turn the switch off so it doesn't spam you continuously
  locationRequested = false;

  // 2. Clean the ESP32 float strings to handle "0.000000" gracefully
  const latNum = parseFloat(lat);
  const lonNum = parseFloat(lon);

  // 3. Build a standard, universally clickable Google Maps link
  const mapLink = (latNum === 0 && lonNum === 0) 
      ? "⚠️ No GPS Lock. Device is active but blind indoors." 
      : `https://maps.google.com/?q=${lat},${lon}`;

  // 4. Send Email and WhatsApp asynchronously in the background
  try {
    // Send Email
    await transporter.sendMail({
      from: process.env.EMAIL_USER,
      to: process.env.EMAIL_USER, 
      subject: "📍 Tracker Update",
      text: `Location Data:\n${mapLink}\nSpeed: ${spd} km/h`
    });

    // Send WhatsApp back to you
    await axios.post(
      `https://api.twilio.com/2010-04-01/Accounts/${ACCOUNT_SID}/Messages.json`,
      new URLSearchParams({
        From: "whatsapp:+14155238886", 
        To: "whatsapp:+918210156811",  // Your target number
        Body: `📍 Location Retrieved:\n${mapLink}\nSpeed: ${spd} km/h`
      }),
      { auth: { username: ACCOUNT_SID, password: AUTH_TOKEN } }
    );
    
    console.log("✅ Alerts dispatched successfully.");

  } catch (error) {
    console.log("❌ ERROR processing alerts.", error.message);
  }
});

// ------------------------------------------------
// START SERVER
// ------------------------------------------------
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`\n🚀 Server running on port ${PORT}`);
});
