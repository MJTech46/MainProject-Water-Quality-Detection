from django.utils import timezone
from zoneinfo import ZoneInfo
from django.shortcuts import render,redirect,HttpResponse
from . import models
from django.http import JsonResponse
from .models import WaterReading, LatestSensorData, SystemMode
from django.views.decorators.csrf import csrf_exempt
import json
import requests
from django.conf import settings
import base64

@csrf_exempt
def ai_chat(request):
    if request.method != "POST":
        return JsonResponse({"error": "POST required"}, status=400)
    messages = json.loads(request.POST.get("messages", "[]"))
    image = request.FILES.get("image")

    # If image uploaded → convert to base64 data URI
    if image:
        img_bytes = image.read()
        img_b64 = base64.b64encode(img_bytes).decode("utf-8")
        mime = image.content_type
        data_uri = f"data:{mime};base64,{img_b64}"
        # Add multimodal message
        messages.append({
            "role": "user",
            "content": [
                {"type": "text", "text": "Analyze this water sample image."},
                {"type": "image_url", "image_url": {"url": data_uri}}
            ]
        })
    payload = {
        "model": "mistralai/ministral-3-3b",
        "messages": messages,
        "temperature": 0.7,
        "max_tokens": 512,
        "stream": False
    }
    response = requests.post(
        "http://localhost:1234/v1/chat/completions",
        headers={"Content-Type": "application/json"},
        data=json.dumps(payload)
    )
    data = response.json()
    reply = data["choices"][0]["message"]["content"]
    return JsonResponse({"reply": reply})

@csrf_exempt
def receive_water_data(request):
    if request.method == "POST":
        temperature = float(request.POST.get("value1"))
        tds = float(request.POST.get("value2"))
        turbidity = float(request.POST.get("value3"))
        level = float(request.POST.get("value4"))
        ri = float(request.POST.get("value5"))
        status = request.POST.get("value6")
        mode = SystemMode.objects.first()
        # If LIVE MODE → store immediately
        if mode.live_mode:
            WaterReading.objects.create(
                temperature=temperature,
                tds=tds,
                turbidity=turbidity,
                water_level=level,
                refactor_index=ri,
                quality_status=status,
                active=True
            )
        # If SINGLE MODE → store temporarily
        else:
            LatestSensorData.objects.update_or_create(
                id=1,
                defaults={
                    "temperature": temperature,
                    "tds": tds,
                    "turbidity": turbidity,
                    "water_level": level,
                    "refactor_index": ri,
                    "quality_status": status
                }
            )
        return JsonResponse({"status": "ok"})

def latest_readings(request):
    readings = WaterReading.objects.filter(
        active=True
    ).order_by('-created_at')[:5]
    readings = reversed(readings)  # Show oldest first
    data = []
    for r in readings:
        data.append({
            "id": r.id,
            "ts": r.created_at,
            "temp": r.temperature,
            "tds": r.tds,
            "turb": r.turbidity,
            "ri": r.refactor_index,
        })
    return JsonResponse(data, safe=False)

@csrf_exempt
def set_mode(request):
    if request.method == "POST":
        mode_value = request.POST.get("mode")
        mode = SystemMode.objects.first()
        # Create if not exists
        if mode is None:
            mode = SystemMode.objects.create(live_mode=True)
        if mode_value == "live":
            mode.live_mode = True
        else:
            mode.live_mode = False
        mode.save()
        return JsonResponse({
            "mode": "live" if mode.live_mode else "single"
        })
    

@csrf_exempt
def collect_reading(request):

    if request.method == "POST":

        latest = LatestSensorData.objects.first()

        if latest:

            WaterReading.objects.create(
                temperature=latest.temperature,
                tds=latest.tds,
                turbidity=latest.turbidity,
                water_level=latest.water_level,
                refactor_index=latest.refactor_index,
                quality_status=latest.quality_status,
                active=True
            )

            return JsonResponse({"status": "collected"})

        return JsonResponse({"status": "no data"})



@csrf_exempt
def manual_reading(request):

    if request.method == "POST":

        temp = float(request.POST.get("temperature"))
        tds = float(request.POST.get("tds"))
        turb = float(request.POST.get("turbidity"))
        ri = float(request.POST.get("ri"))

        WaterReading.objects.create(
            temperature=temp,
            tds=tds,
            turbidity=turb,
            water_level=0,
            refactor_index=ri,
            quality_status="MANUAL",
            active=True
        )

        return JsonResponse({"status":"saved"})


# Create your views here.
def index(request):
    return render(request, 'index.html')


def register(request):
    if request.method == 'POST':
        username = request.POST.get('username')
        age = request.POST.get('age')
        gender = request.POST.get('gender')
        email = request.POST.get('email')
        phone = request.POST.get('phone')
        address = request.POST.get('address')
        password = request.POST.get('password')
        image = request.FILES.get('image')

        if models.Register.objects.filter(email=email).exists():
           alert="<script>alert('Email already exists!');window.location.href='/register/';</script>;"
           return HttpResponse(alert)
        
        else:
            user = models.Register(username=username,age=age,gender=gender,email=email,phone=phone,address=address,password=password,image=image)
            user.save()
            return redirect('login')
    return render(request, 'register.html')


def login(request):
    if request.method == 'POST':
        email = request.POST.get('email')
        password = request.POST.get('password')
        try:
            user = models.Register.objects.get(email=email)

            if user.password == password:
                request.session['email'] = email
                return redirect('userhome')
            else:
                return HttpResponse("<script>alert('Invalid email or password!');window.location.href='/login/';</script>;")
            
        except models.Register.DoesNotExist:
            return HttpResponse("<script>alert('User Not Found!');window.location.href='/login/';</script>;")
        
    return render(request, 'login.html')


def userhome(request):
    return render(request, 'userhome.html')




def logout(request):
    request.session.flush()
    return redirect('index')

def profile(request):
    if 'email' in request.session:
        email = request.session['email']
        try:
            user = models.Register.objects.get(email=email)
            return render(request, 'profile.html', {'user': user})
        
        except models.Register.DoesNotExist:
            return HttpResponse("<script>alert('User Not Found!');window.location.href='/login/';</script>;")
        
    return redirect('login')


def editprofile(request):
    if 'email' in request.session:
        email = request.session['email']

        try:
            user = models.Register.objects.get(email=email)

            if request.method == 'POST':
                user.username = request.POST.get('username')
                user.age = request.POST.get('age')
                user.gender = request.POST.get('gender')
                user.email = request.POST.get('email')
                user.phone = request.POST.get('phone')
                user.address = request.POST.get('address')
                user.image = request.FILES.get('image', user.image)
                user.save()
                return HttpResponse("<script>alert('Profile updated successfully!');window.location.href='/profile/';</script>;")

            return render(request, 'editprofile.html', {'user': user})

        except models.Register.DoesNotExist:
            return HttpResponse("<script>alert('User Not Found!');window.location.href='/login/';</script>;")

    return redirect('login')    