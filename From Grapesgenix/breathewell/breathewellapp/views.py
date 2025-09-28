from django.shortcuts import render,redirect,HttpResponse
from urllib3 import request
from . import models

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