from django.test import TestCase, Client
from unittest.mock import patch
import json

from .models import SystemMode


class WaterQualityAPITest(TestCase):

    def setUp(self):
        self.client = Client()

        #print("\n⚙️ Setting up test environment...")

        # Ensure mode exists
        SystemMode.objects.create(live_mode=True)

        #print("✅ SystemMode initialized\n")

    # 1️⃣ Test water-data API (ESP32 sending data)
    def test_receive_water_data(self):

        payload = {
            "value1": 27.4,
            "value2": 356,
            "value3": 88,
            "value4": 10,
            "value5": 82,
            "value6": "GOOD"
        }

        print("\n📡 TEST: ESP32 sending water data")
        print("➡️ Data Sent:", payload)

        response = self.client.post("/api/water-data/", payload)

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Water data API working\n")

    # 2️⃣ Test latest readings API
    def test_latest_readings(self):

        print("\n📊 TEST: Fetch latest readings")

        response = self.client.get("/api/latest-readings/")

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Latest readings API working\n")

    # 3️⃣ Test set mode API
    def test_set_mode_live(self):

        payload = {"mode": "live"}

        print("\n⚙️ TEST: Set system mode to LIVE")
        print("➡️ Data Sent:", payload)

        response = self.client.post("/api/set-mode/", payload)

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Mode set to LIVE successfully\n")

    def test_set_mode_single(self):

        payload = {"mode": "single"}

        print("\n⚙️ TEST: Set system mode to SINGLE")
        print("➡️ Data Sent:", payload)

        response = self.client.post("/api/set-mode/", payload)

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Mode set to SINGLE successfully\n")

    # 4️⃣ Test manual reading API
    def test_manual_reading(self):

        payload = {
            "temperature": 27.4,
            "tds": 356,
            "turbidity": 88,
            "ri": 82
        }

        print("\n🧪 TEST: Manual water reading submission")
        print("➡️ Data Sent:", payload)

        response = self.client.post("/api/manual-reading/", payload)

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Manual reading API working\n")

    # 5️⃣ Test collect-reading API
    def test_collect_reading(self):

        print("\n📥 TEST: Collect reading trigger")

        response = self.client.post("/api/collect-reading/")

        print("⬅️ Response Status:", response.status_code)
        print("⬅️ Response Data:", response.content.decode())

        self.assertEqual(response.status_code, 200)

        print("✅ Collect reading API working\n")

    # # 6️⃣ Test AI chat API (Mocked)
    # @patch("breathewellapp.views.requests.post")
    # def test_ai_chat(self, mock_post):

    #     print("\n🤖 TEST: AI Chat API")

    #     # Mock AI response
    #     mock_post.return_value.json.return_value = {
    #         "choices": [
    #             {
    #                 "message": {
    #                     "content": "Water quality looks acceptable."
    #                 }
    #             }
    #         ]
    #     }

    #     payload = {
    #         "messages": [
    #             {
    #                 "role": "user",
    #                 "content": "Analyze this water sample: TDS 356 ppm, Turbidity 88%, RI 82"
    #             }
    #         ]
    #     }

    #     print("➡️ Data Sent to AI:", payload)

    #     response = self.client.post(
    #         "/api/ai-chat/",
    #         data=json.dumps(payload),
    #         content_type="application/json"
    #     )

    #     print("⬅️ Response Status:", response.status_code)
    #     print("⬅️ AI Response:", response.content.decode())

    #     self.assertEqual(response.status_code, 200)

    #     print("✅ AI Chat API working\n")