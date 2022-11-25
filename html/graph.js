var xValues = [];
var yValues = [];
var counter = 0;

while(counter < 100){
    xValues.push(counter);
    yValues.push(Math.sin(counter*Math.PI/20));
    counter+=1;
}

function displayChart(){
    if(checker){
        counter+=1;

        xValues.push(counter);
        xValues.shift();
        yValues.push(Math.sin(counter*Math.PI/20));
        yValues.shift();

        gtd.data.labels = xValues;
        gtd.data.datasets.data = yValues;
        gtd.options.scales.yAxes = [{ticks: {min: -1, max: 1}}]
        gtd.update();
    }
};

var gtd = new Chart(
    "graph",
    {
        type: "line",
        data: {
            labels: xValues,
            datasets: [{
                backgroundColor: "rgba(0,0,50,0.5)",
                borderColor: "rgba(0,0,50,1.0)",
                data: yValues
            }]
        },
        options: {
            legend: {display: false},
            scales: {
                yAxes: [{ticks: {min: -1, max: 1}}],
            },
            animation: false,
            decimation: true
        }
    }
);