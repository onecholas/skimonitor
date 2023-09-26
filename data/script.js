function get_color() {
    var color = (document.getElementById("colorpicker").value).slice(1,);
    console.log("Color: " + color);
    console.log("Red: " + parseInt(color.slice(0,2), 16));
    console.log("Green: " + parseInt(color.slice(2,4), 16));
    console.log("Blue: " + parseInt(color.slice(4,6), 16));
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/update?output=colorpickerred&state="+parseInt(color.slice(0,2), 16), true);
    xhr.send();
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/update?output=colorpickergreen&state="+parseInt(color.slice(2,4), 16), true);
    xhr.send();
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/update?output=colorpickerblue&state="+parseInt(color.slice(4,6), 16), true);
    xhr.send();
}

function color_mode() {
    var selected = document.querySelector('input[name="option"]:checked').id;
    // console.log(selected);
    if ((selected === "option5" || selected === "option6" || selected === "option7" || selected === "option8") && document.getElementById("onoff").checked) {
        document.getElementById("colorpicker").disabled = false;
    }
    else {
        document.getElementById("colorpicker").disabled = true;
    }
}
function load_speed(element) {
    console.log("LOAD_SPEED CALLED");
    // Check if the data is already set in sessionStorage
    if (!sessionStorage.getItem(element)) {
        console.log("First page visit");
        sessionStorage.setItem(element, "mph");
    }
    else {
        console.log("Not first page visit");
        var curr = document.getElementById(element);
        curr.innerHTML = sessionStorage.getItem(element);
    }
}
function toggle_speed(element) {
    var curr = document.getElementById(element);
    if (curr.innerHTML === "mph") {
        sessionStorage.setItem(element, "kph");
        curr.innerHTML = "kph";
    }
    else if (curr.innerHTML === "kph") {
        sessionStorage.setItem(element, "m/s");
        curr.innerHTML = "m/s";
    }
    else if (curr.innerHTML === "m/s") {
        sessionStorage.setItem(element, "ft/s");
        curr.innerHTML = "ft/s";
    }
    else {
        sessionStorage.setItem(element, "mph");
        curr.innerHTML = "mph";
    }
}
function load_distance(element) {
    console.log("LOAD_DISTANCE CALLED")
    // Check if the data is already set in sessionStorage
    if (!sessionStorage.getItem(element)) {
        console.log("First page visit");
        sessionStorage.setItem(element, "mi");
    }
    else {
        console.log("Not first page visit");
        var curr = document.getElementById(element);
        curr.innerHTML = sessionStorage.getItem(element);
    }
}
function toggle_distance(element) {
    var curr = document.getElementById(element);
    if (curr.innerHTML === "m") {
        sessionStorage.setItem(element, "km");
        curr.innerHTML = "km";
    }
    else if (curr.innerHTML === "km") {
        sessionStorage.setItem(element, "ft");
        curr.innerHTML = "ft";
    }
    else if (curr.innerHTML === "ft") {
        sessionStorage.setItem(element, "mi");
        curr.innerHTML = "mi";
    }
    else {
        sessionStorage.setItem(element, "m");
        curr.innerHTML = "m";
    }
}
function load_temperature(element) {
    console.log("LOAD_TEMPERATURE CALLED")
    // Check if the data is already set in sessionStorage
    if (!sessionStorage.getItem(element)) {
        console.log("First page visit");
        sessionStorage.setItem(element, "°F");
    }
    else {
        console.log("Not first page visit")
        var curr = document.getElementById(element);
        curr.innerHTML = sessionStorage.getItem(element)
    }
}
function toggle_temperature(element) {
    var curr = document.getElementById(element);
    if (curr.innerHTML === "°F") {
        sessionStorage.setItem(element, "°C");
        curr.innerHTML = "°C";
    }
    else {
        sessionStorage.setItem(element, "°F");
        curr.innerHTML = "°F";
    }
}
function check_ON() {
    var x = document.getElementById("onoff").checked;
    var radioButtons = document.querySelectorAll('input[type="radio"]');
    var buttonLabels = document.querySelectorAll("#buttonLabel");
    radioButtons.forEach(radio => {radio.disabled = !x;});
    if (x === false) { 
        document.getElementById("LED Status").innerHTML = "OFF";
        buttonLabels.forEach(button => {button.style.color = "#ddd9d9";});
    }
    else {
        // document.getElementById("option0").checked = true;
        document.getElementById("LED Status").innerHTML = "ON";
        buttonLabels.forEach(button => {button.style.color = "#444444";});
    }
}
function timer_button() {
    var x = document.getElementById("timer").checked;
    console.log("Timer: " + x);
    if (x) {
        document.getElementById("timer-control").textContent = "Stop Run";
        document.getElementById("timer-control").style.background = "rgb(206, 112, 112)";
    }
    else {
        document.getElementById("timer-control").textContent = "Start Run";
        document.getElementById("timer-control").style.background = "#5ab36d";
    }
}
function ski_mode(element) {
    var xhr = new XMLHttpRequest();
    if(element.checked){ 
        xhr.open("GET", "/update?output="+element.id+"&state=1", true);
    }
    else {
        xhr.open("GET", "/update?output="+element.id+"&state=0", true);
    }
    xhr.send();
}
function display_data(elementtoupdate, dataattribute, elementunits) {
    var element = document.getElementById(elementtoupdate);
    var dataValue = element.getAttribute(dataattribute);
    if (elementunits !== null) {
        var units = document.getElementById(elementunits).innerHTML
    }
    else {
        var units = null;
    }
    if (element.id === "battery_status" || element.id === "gps_status") {
        element.innerHTML = dataValue;
        console.log(element);
        console.log(dataValue);
    }
    else if (element.id === "curr_time_value" || element.id === "runs_completed" || element.id === "avg_run_time" || element.id === "total_run_time") {
        var regex = /%[^%]+%/;
        if (!regex.test(dataValue)) {
            element.innerHTML = dataValue;
            console.log(element);
            console.log(dataValue);
        }
        else {
            element.innerHTML = "N/A";
        }
    }
    else {
        var regex = /\d/;
        if (regex.test(dataValue)) {
            if (element.id === "curr_speed_value" || element.id === "curr_top_speed_value" || element.id === "overall_top_speed_value") {
                dataValue = parseFloat(dataValue);
                if (units === "mph") {
                    dataValue = dataValue * 2.237;
                }
                else if (units === "kph") {
                    dataValue = dataValue * 3.6;
                }
                else if (units === "ft/s") {
                    dataValue = dataValue * 3.281;
                }
            }
            if (element.id === "curr_dist_value" || element.id === "curr_elev_value" || element.id === "overall_dist_value" || element.id === "overall_elev_value") {
                dataValue = parseFloat(dataValue);
                if (units === "km") {
                    dataValue = dataValue * 0.001;
                }
                else if (units === "ft") {
                    dataValue = dataValue * 3.28;
                }
                else if (units === "mi") {
                    dataValue = dataValue * 0.000621;
                }
            }
            if (element.id === "curr_temp_value") {
                dataValue = parseFloat(dataValue);
                if (units === "°F") {
                    dataValue = dataValue * 9/5 + 32;
                }
            }
            dataValue = dataValue.toString();
            if (dataValue.length > 4 ) {
                dataValue = dataValue.slice(0,4);
            }
            element.innerHTML = dataValue;
            console.log(element);
            console.log(dataValue);            
        }
        else {
            element.innerHTML = "N/A";
        }
    }
}
function update_data(datatoupdate, dataattribute, data) {
    const element = document.getElementById(datatoupdate);
    element.setAttribute(dataattribute, data);
}

var speedChart = new Highcharts.Chart({
    chart:{ renderTo : 'top_speed_chart',
            maxHeight: 50},
    title: { text: 'Top Speed VS Run' },
    series: [{
        showInLegend: false,
        data: []
        
    }],
    plotOptions: {
        line: { animation: false,
        dataLabels: { enabled: true }
        },
        series: { color: '#059e8a' }
    },
    xAxis: {
        title: { text: 'Run'}
    },
    yAxis: {
        title: { text: 'Top Speed (mph)' }
    },
    credits: { enabled: false }
});

function parse_and_post_graph_data(data) {
    var [run, top_speed] = data.split(" ");
    run = parseFloat(run);
    top_speed = parseFloat(top_speed);
    //console.log(this.responseText);
    if(speedChart.series[0].data.length > 100) {
    speedChart.series[0].addPoint([run, top_speed], true, true, true);
    } else {
    speedChart.series[0].addPoint([run, top_speed], true, false, true);
    }
}

if (!!window.EventSource) {
    var source = new EventSource('/events');
    // Default event listeners
    source.addEventListener('open', function(e) { console.log("Events Connected"); }, false);
    source.addEventListener('error', function(e) { if (e.target.readyState != EventSource.OPEN) { console.log("Events Disconnected"); }}, false);
    source.addEventListener('message', function(e) { console.log("message", e.data); }, false);

    // BMP event listeners
    source.addEventListener('pressure', function(e) { console.log("pressure", e.data); }, false);
    source.addEventListener('temperature', function(e) { console.log("temperature", e.data); update_data("curr_temp_value", "data-curr-temp", e.data); }, false);
    // source.addEventListener('altitude', function(e) { console.log("altitude", e.data); jQuery("#curr_elev_value").data("curr-elev",e.data); }, false);

    // Timer event listener
    source.addEventListener('time', function(e) { console.log("time", e.data); update_data("curr_time_value", "data-curr-time", e.data); }, false);

    // NEO6M event listeners
    source.addEventListener('runvelocity', function(e) { console.log("run velocity", e.data); update_data("curr_speed_value", "data-curr-speed", e.data); }, false);
    source.addEventListener('toprunvelocity', function(e) { console.log("top run velocity", e.data); update_data("curr_top_speed_value", "data-curr-top-speed", e.data); }, false);
    source.addEventListener('elevationdiffgps', function(e) { console.log("elevation difference gps", e.data); update_data("curr_elev_value", "data-curr-elev", e.data); }, false);
    source.addEventListener('rundist', function(e) { console.log("run distance", e.data); update_data("curr_dist_value", "data-curr-dist", e.data); }, false);

    // Battery event listener
    source.addEventListener('battery', function(e) { console.log("battery", e.data); update_data("battery_status", "data-battery-status", e.data); }, false);
    source.addEventListener('gps', function(e) { console.log("gps", e.data); update_data("gps_status", "data-gps-status", e.data); }, false);

    // Other
    source.addEventListener('overalltoprunvel', function(e) { console.log("overall run velocity", e.data); update_data("overall_top_speed_value", "data-overall-top-speed", e.data); }, false);
    source.addEventListener('overalldist', function(e) { console.log("overall distance", e.data); update_data("overall_dist_value", "data-overall-dist", e.data); }, false);
    source.addEventListener('overallelev', function(e) { console.log("overall elevation", e.data); update_data("overall_elev_value", "data-overall-elev", e.data); }, false);
    source.addEventListener('totalruns', function(e) { console.log("overall runs", e.data); update_data("runs_completed", "data-runs-completed", e.data); }, false);
    source.addEventListener('avgtime', function(e) { console.log("overall average run time", e.data); update_data("avg_run_time", "data-avg-run-time", e.data); }, false);
    source.addEventListener('totaltime', function(e) { console.log("overall run time", e.data); update_data("total_run_time", "data-total-run-time", e.data); }, false);

    // Graphing
    source.addEventListener('graphtoprunvel', function(e) { console.log("graph top run vel", e.data); parse_and_post_graph_data(e.data); }, false);
}

function rotate_bar_needle() {
    // Max Speed: 70 mph = 31.29 m/s. Thus, 180 degrees total -> each increment of 1 m/s is ~= 5.7526 degrees
    var rotation = (document.getElementById('curr_speed_value').getAttribute("data-curr-speed")) * 180/31.29 + 45;
    document.getElementById("curr_speed_bar").style.transform = "rotate(" + rotation + "deg)";
    rotation -= 45;
    document.getElementById("speed_needle").style.transform = "rotate(" + rotation + "deg)";
}
var observer = new MutationObserver(rotate_bar_needle);
observer.observe(document.getElementById('curr_speed_value'), {characterData: true, attributes: true, childList: true, subtree: true});


setInterval(
    function () {  
        display_data("curr_time_value", "data-curr-time", null); //
        // console.log("Updated curr_time_value");

        display_data("curr_speed_value", "data-curr-speed", "curr_speed_unit"); //
        // console.log("Updated curr_speed_value");

        display_data("curr_top_speed_value", "data-curr-top-speed", "curr_top_speed_unit"); //
        // console.log("Updated curr_top_speed_value");

        display_data("curr_dist_value", "data-curr-dist", "curr_dist_unit"); //
        // console.log("Updated curr_dist_value");

        display_data("curr_elev_value", "data-curr-elev", "curr_elev_unit"); //
        // console.log("Updated curr_elev_value");

        display_data("curr_temp_value", "data-curr-temp", "curr_temp_unit"); //
        // console.log("Updated curr_temp_value");

        display_data("overall_top_speed_value", "data-overall-top-speed", "top_speed_unit"); //
        // console.log("Updated overall_top_speed_value");

        display_data("overall_dist_value", "data-overall-dist", "overall_dist_unit"); //
        // console.log("Updated overall_dist_value");

        display_data("overall_elev_value", "data-overall-elev", "overall_elev_unit"); //
        // console.log("Updated overall_elev_value");

        display_data("runs_completed", "data-runs-completed", null); //
        // console.log("Updated runs_completed");

        display_data("avg_run_time", "data-avg-run-time", null);
        // console.log("Updated runs_completed");

        display_data("total_run_time", "data-total-run-time", null);
        // console.log("Updated total_run_time");

        display_data("battery_status", "data-battery-status", null);

        display_data("gps_status", "data-gps-status", null);  

        // speedChart.reflow();
    }
    , 500
);
