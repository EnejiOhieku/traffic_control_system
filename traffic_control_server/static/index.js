
traffic_canvas = document.getElementById("card0")
interval_id = 0

road_lights = []
density_bars = []

for (i=0; i<3; i++) {
    road_light = []
    pos = ["left: 250px; top: 60px;", "left: 420px; top: 60px;", "left: 275px; top: 190px;"]

    for (j=0; j<3; j++) {
        road_light.push(`
        <div style="position: relative;
                    width: 40px;
                    height: 100px;
                    ${pos[i]}
                    background-image: url('/static/traffic_light/${j}.png');
                    background-size: cover;">
                    </div>`)
    }
    road_lights.push(road_light)


    density_bar = []
    pos = ["top: 50px; left: -215px;", "top: 50px; left: 40px;", "top: 180px; left: -290px;"]
    for (j=0; j<4; j++) {
        density_bar.push(`
        <div style="position: relative;
                    width: 90px;
                    ${pos[i]}
                    background-image: url('/static/density_bar/${j}.png');
                    background-size: contain;
                    background-repeat: no-repeat;">
                    </div>`)
    }
    density_bars.push(density_bar)
}




function setCard(index) {
    for (i=0; i<2; i++) {
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

function setTrafficLight(state, density, count) {
    content = ``
    for (i=0; i<3; i++) {
        content += road_lights[i][state[i]]
    }

    info_pos = ["left: 40px; top: 60px;", "left: 300px; top: 60px;", "left: -25px; top: 190px;"]
    for (i=0; i<3; i++) {
        content += `<div class="sm-card" style="position: relative; ${info_pos[i]}">Road: ${i+1}<br>Count: ${count[i]}</div>`
    }

    for (i=0; i<3; i++) {
        content += density_bars[i][density[i]]
    }
    traffic_canvas.innerHTML = content
}

function setTableData(table, last_time) {
    num_col = table.length
    num_row = table[0].length
    table_dom = document.getElementById("data_table")


    header_td = ""
    for (i=last_time; i>last_time-num_col; i--) {
        header_td = `<th>${String((24+i)%24).padStart(2, "0")}:00</th>` + header_td
    }
    header = `<thead>
                <tr>
                    <th>Time</th>
                    ${header_td}
                    <th>Total</th>
                </tr>
            </thead>`


    row_total = Array(num_row).fill(0)
    col_total = Array(num_col).fill(0)


    rows_td = ""
    for (i=0; i<num_row; i++) {
        row_td = `<td>Road ${i+1}</td>`
        for (j=0; j<num_col; j++) {
            row_td += `<td>${table[j][i]}</td>`
            row_total[i] += table[j][i]
            col_total[j] += table[j][i]

            if (j == num_col - 1) {
                row_td += `<td>${row_total[i]}</td>`
            }
        }
        rows_td += `<tr>${row_td}</tr>`
    }
    body = `<tbody>${rows_td}</tbody>`

    footer_td = ""
    for (i=0; i<num_col; i++) {
        footer_td += `<td>${col_total[i]}</td>`
    } 
    footer = `<tfoot>
                <td>Total</td>
                ${footer_td}
                <td>${row_total.reduce((acc, e) => acc+=e)}</td>
            </tfoot>`


    table_dom.innerHTML = header + body + footer

}

function updatePanel() {
    fetch(`/pull`)
    .then((response)=> {return response.json()})
    .then((json) => {
            document.getElementById("last_update").innerText = `Last Update: ${json["last_update"]}`
            setTrafficLight(json["states"], json["densities"], json["count"])
            setTableData(json["traffic"], json["last_time"])
        }
    )
}


function setPeriod() {
    input = document.getElementById("refresh_period")
    period = Number(input.value)
    clearInterval(interval_id)
    interval_id = setInterval(updatePanel, period)
}


interval_id = setInterval(updatePanel, 1000)

