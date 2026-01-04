const ctx = document.getElementById("myChart") as HTMLCanvasElement;

let online = false;

let chartData: { labels: number[]; tempData: number[]; humidityData: number[] } = {labels: [], tempData: [], humidityData: []
}

const resp = await fetch("/data.json");

const data: {
    latestSample: number;
    hourSamples: [number, number][];
    quarterHourSamples: [number, number][];
} = await resp.json();

let curTimeStep = data.latestSample * 1000
let tempLabels : number[] = []

let tempHumiditys : number[]= []
let tempTemperatures : number[]= []


while(data.quarterHourSamples.length != 0){
    let value = data.quarterHourSamples.pop()
    if (!value){
        break
    }
    tempLabels.push(curTimeStep)
    curTimeStep-= 15*60*1000
    tempHumiditys.push(value[0])
    tempTemperatures.push(value[1])
}

while(data.hourSamples.length != 0){
    let value = data.hourSamples.pop()
    if (!value){
        break
    }
    tempLabels.push(curTimeStep)
    curTimeStep-= 60*60*1000
    tempHumiditys.push(value[0])
    tempTemperatures.push(value[1])
}

chartData.labels = tempLabels.reverse()
chartData.humidityData = tempHumiditys.reverse()
chartData.tempData = tempTemperatures.reverse()


try {
    var mainChart = new Chart(ctx, {
        type: "line",

        data: {
            labels: chartData.labels,
            datasets: [
                {
                    label: "Relative Humidity (%)",
                    data: chartData.humidityData,
                    borderWidth: 2,
                    yAxisID: "H",
                    borderColor: "#7891c5ff",
                },
                {
                    label: "Temperature (C)",
                    data: chartData.tempData,
                    borderWidth: 2,
                    yAxisID: "T",
                    borderColor: "#ce7b67ff",
                },
            ],
        },
        options: {
            scales: {
                H: {
                    position: "left",
                    min: 50,
                    max: 100,
                    grid: {
                        color: "#5e76a8ff", // grid line color
                    },
                    ticks: {
                        color: "#adbbdaff", // tick text color
                    },
                    title: {
                        display: true,
                        text: "Humidity (%)",
                        color: "#adbbdaff",
                    },
                },
                T: {
                    position: "right",
                    min: 15,
                    max: 30,
                    grid: {
                        color: "#975845ff", // grid line color
                    },
                    ticks: {
                        color: "#e98043ff", // tick text color
                    },
                    title: {
                        display: true,
                        text: "Humidity (%)",
                        color: "#e98043ff",
                    },
                },
                x: {
                    type: "time",
                    axis: "x",
                    time: {
                        minUnit: "minute",
                    },
                    grid: {
                        display: false,
                    },
                    ticks: {
                        color: "#cccccc", // tick text color
                    },
                },
            },
        },
    });

    online = true;
} catch {
    online = false;

    document.getElementById("online")?.remove();
}

export {};
