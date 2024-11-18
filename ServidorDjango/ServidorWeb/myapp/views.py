from django.shortcuts import render
from django.http import HttpResponse, JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json
from . models import datosCrawler
# Create your views here.
#Lo que se envía al cliente


def hello(request):
    return HttpResponse("<h1> Hello world </h1>")

def about(request):
    return HttpResponse("About")

@csrf_exempt
def recibir_dato(request):
    if request.method=='POST':
        try:
            data=json.loads(request.body)
            
            dato=data.get('dato')
            valor=data.get('valor')
            print(dato)
            printf(valor)
            print(data)
            nuevo_dato= datosCrawler(dato=dato, valor=valor)
            nuevo_dato.save()
            response_data={
                'id':nuevo_dato.id,
                'dato':nuevo_dato.valor,
                'valor':nuevo_dato.valor,
                'timestamp':nuevo_dato.timestamp
            }
            
            
            return JsonResponse({'status': 'success', 'dato_recibido': response_data}, status=200)
        except json.JSONDecodeError:
            return JsonResponse({'status':'error', 'message': 'Invalid JSON'}, status=400)
    else:
        return JsonResponse({'status': 'error', 'message': 'Solo acepta POST requests'}, status= 405)