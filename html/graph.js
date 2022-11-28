var hr_xValues = [];
var hr_yValues = [];
var spo2_xValues = [];
var spo2_yValues = [];
var counter = 0;

while(counter < 100){
    hr_xValues.push(0);
    hr_yValues.push(0);
    spo2_xValues.push(0);
    spo2_yValues.push(0);
    counter+=1;
}

function displayChart(){
    try{
        counter+=1;
        const d = new Date();

        hr_yValues.push(heart_rate);
        hr_yValues.shift();
        hr_xValues.push(d.getTime()/1000);
        hr_xValues.shift;

        spo2_yValues.push(spo2_rate);
        spo2_yValues.shift();
        spo2_xValues.push(d.getTime()/1000);
        spo2_xValues.shift;

        gtd.data.labels = hr_xValues;
        gtd.data.datasets.data = hr_yValues;
        gtd.update();

        std.data.labels = spo2_xValues;
        std.data.datasets.data = spo2_yValues;
        std.update();
    }
    catch(error){console.error(`Something went wrong. ${error}`); document.getElementById("errorLog").innerHTML = error;};
};

var gtd = new Chart(
    "hr_graph",
    {
        type: "line",
        data: {
            labels: hr_xValues,
            datasets: [{
                backgroundColor: "rgba(0,0,50,0.5)",
                borderColor: "rgba(0,0,50,1.0)",
                data: hr_yValues
            }]
        },
        options: {
            legend: {display: false},
            scales: {
                yAxes: [{ticks: {min: 40, max: 180}}],
            },
            animation: false,
            decimation: true
        }
    }
);

var std = new Chart(
    "spo2_graph",
    {
        type: "line",
        data: {
            labels: spo2_xValues,
            datasets: [{
                backgroundColor: "rgba(0,0,50,0.5)",
                borderColor: "rgba(0,0,50,1.0)",
                data: spo2_yValues
            }]
        },
        options: {
            legend: {display: false},
            scales: {
                yAxes: [{ticks: {min: 0, max: 100}}],
            },
            animation: false,
            decimation: true
        }
    }
);