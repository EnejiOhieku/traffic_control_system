roads_light = []
traffic_canvas = document.getElementById("card0")

for (i=0; i<3; i++) {
    images = []
    pos = {
        0: "left: 250px; top: 60px;",
        1: "left: 420px; top: 60px;",
        2: "left: 275px; top: 160px;",
    }
    for (j=0; j<3; j++) {
        images.push(`position: relative; width: 40px; height: 100px; ${pos[j]} background-image: url('/static/traffic_light/${j}.png'); background-size: cover;`)
    }
    roads_light.push(images)
}

console.log(roads_light)
function setCard(index) {
    for (i=0; i<3; i++) {
        btn = document.getElementById(`btn${i}`)
        btn.classList.remove("button-active")
        
        card = document.getElementById(`card${i}`)
        card.style.display = "none"
        if (index == i) {
            btn.classList.add("button-active")
            card.style.display = "flex"
        }
    }
}

function setTrafficLight(roads) {
    traffic_canvas
}