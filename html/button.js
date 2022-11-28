var device;
var service;
var characteristic;

async function connectToOxiband(){
    let options = {
        filters: [
            { services: [0x5a30] },
            { name: "OxiClip" }
        ],
    };
    
    try{
        device = await navigator.bluetooth.requestDevice(options);
        gatt = await device.gatt.connect();
        service = await gatt.getPrimaryService(0x5a30);
        heart_rate_char = await service.getCharacteristic(0x7a21);
        spo2_rate_char = await service.getCharacteristic(0x7a23);
        time_calibrate = await service.getCharacteristic(0x7a25);
        console.log('> Successfully established hook-up with ' + device.name);
        document.getElementById("status").innerHTML = " Connected";
        device.addEventListener('gattserverdisconnected', onDisconnected);
    }
    catch(error){console.error(`Something went wrong. ${error}`); document.getElementById("errorLog").innerHTML = error;};
}

function onDisconnected(){
    console.log('> Bluetooth device disconnected');
    document.getElementById("status").innerHTML = " Not connected";
}

var heart_rate
var spo2_rate
var last_time

function checkCharacteristics(){
    try{
        if (device) {
            time_calibrate.readValue().then(
                package => {
                    if(package.getUint8(0)!=last_time){
                        last_time = package.getUint8(0);
                        heart_rate_char.readValue().then(
                            heart_package => {
                                heart_rate = heart_package.getUint8(0);
                            }
                        )
                        spo2_rate_char.readValue().then(
                            spo2_package => {
                                spo2_rate = spo2_package.getUint8(0);
                            }
                        )
                        displayChart();
                    }
                }
            )
        }
    }
    catch(error){console.error(`Something went wrong. ${error}`); document.getElementById("errorLog").innerHTML = error;};
}