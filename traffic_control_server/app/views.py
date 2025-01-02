from django.http import HttpRequest, JsonResponse
from django.shortcuts import render
import json
import os
import time
import datetime

# Create your views here.

"""
    Json format for data push from microcontroller 
    {
    "states": [0, 1, 2],
    "densities": [3, 0, 1],
    "count": [12, 1, 3],
}
"""


file_path = os.path.join(os.getcwd(), "app/data.json")


def push_data(request: HttpRequest):
    try:
        if request.method == "GET":
            received_data = json.loads(request.GET["data"])

            with open(file_path) as jsonData:
                stored_data = json.load(jsonData)

                diff = int((time.time() - stored_data["last_update"]) // (3600))
                if diff > 24: diff = 24

                stored_data["states"] = received_data["states"]
                stored_data["densities"] = received_data["densities"]
                stored_data["traffic"] = stored_data["traffic"][diff:] + ([[0, 0, 0]] * diff)
                stored_data["traffic"][-1] = received_data["count"]
                stored_data["last_update"] = time.time()

            with open(file_path, "w") as jsonData:
                json.dump(stored_data, jsonData) 
        return JsonResponse({"success": True})
    except:
        return JsonResponse({"success": False})



def pull_data(request: HttpRequest):
    try:
        with open(file_path) as jsonData:
            response = json.load(jsonData)
        
        time_diff = int(time.time() - response["last_update"])

        if time_diff > 24*3600:
            formatted_time = "More than a Day"
        else:
            hrs = time_diff//3600
            mins = (time_diff - hrs*3600)//60
            secs = time_diff % 60
            formatted_time = f"{hrs}Hrs {mins}min {int(secs)}secs"

        response["last_update"] = formatted_time
        response["success"] = True
        return JsonResponse(response)
    except:
        return JsonResponse({"success": False})
    
    
def home(request: HttpRequest):
    return render(request, "index.html")