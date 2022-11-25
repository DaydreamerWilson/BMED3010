var checker = false;

function startButton(){
    document.getElementById("notify").innerHTML = "You pressed the start button";
    checker = true;
}

function stopButton(){
    document.getElementById("notify").innerHTML = "You pressed the stop button";
    checker = false;
}