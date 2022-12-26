updateGauges = (obj) =>{
  var newTime = Date.now()+3600000;
  var usedPwr = ((obj.ina2.wattage)/1000).toFixed(2);
  var generatedPwr = ((obj.ina1.wattage)/1000).toFixed(2);
  var bat = (obj.ina1.voltage).toFixed(2);
  var ratio = ((generatedPwr / usedPwr)*100).toFixed(2);
  document.querySelector("#usedpwr").innerText = usedPwr;
  document.querySelector("#generatedpwr").innerText = generatedPwr;
  document.querySelector("#ratio").innerText = ratio;
  document.querySelector("#bat").innerText = bat;
  solardata.push([newTime,generatedPwr]);
  espdata.push([newTime,usedPwr]);
  var solarmax = 0;
  var espmax = 0;
  solardata.map(d => {
    solarmax = Math.max(solarmax, d[1])
  });
  espdata.map(d => {
    espmax = Math.max(espmax, d[1])
  });
  espchart.updateOptions({
    yaxis: {
      max: (((espmax > solarmax) ? espmax : solarmax)+1).toFixed(0)*1
    }
  });


  espchart.updateSeries([{
    data: espdata
  },
  {
    data: solardata
  }]);
};





fetchData = () => {
    fetch('data', {
        method: 'GET',
        headers: {
            'Accept': 'application/json'
        },
    })
    .then(response => response.json())
    .then(text => {
        updateGauges(text);

    })
};

window.onload = function() {
    document.getElementsByClassName("mask")[0].style = "visibility: hidden";;
    setInterval(fetchData, 1000);
};


function getNewData(baseval, yrange) {
  var newTime = baseval+3600000;
  return {
    x: newTime,
    y: Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min
  };
};

window.Apex = {
  chart: {
    foreColor: "#fff",
    toolbar: {
      show: false
    }
  },
  colors: ["#e63946", "#2ec4b6", "#f02fc2"],
  stroke: {
    width: 5
  },
  dataLabels: {
    enabled: false
  },
  grid: {
    borderColor: "#40475D"
  },
  xaxis: {
    axisTicks: {
      color: "#333"
    },
    axisBorder: {
      color: "#333"
    }
  },
  tooltip: {
    theme: "dark",
    x: {
      formatter: function (val) {
        var d = new Date(val);
        var min = () => {
          if (d.getUTCMinutes()<10){
            return "0"+d.getUTCMinutes();
          }
          else return d.getUTCMinutes();
        };
        var parsed = d.getUTCHours()+":"+min()+":"+d.getUTCSeconds();
        return parsed;
      }
    }
  }
};



var espdata = [];
var solardata = [];
var EspOptions = {
  series: [{
    name: "Used Power",
    data: espdata.slice()
},
  {
    name: "Generated Power",
    data: solardata.slice()
  }
],
  chart: {
    id: 'realtime',
    height: 400,
    type: 'line',
    fontFamily: 'Rubik, Verdana, sans-serif',
    animations: {
      enabled: true,
      easing: 'linear',
      dynamicAnimation: {
        speed: 1000
      }
    },
    toolbar: {
      show: false
    },
    zoom: {
      enabled: false
    }
  },
  dataLabels: {
    enabled: false
  },
  stroke: {
    curve: 'smooth'
  },
  title: {
    text: 'Used power',
    align: 'left',
    color: '#EDF2F4',
    style: {
      fontSize: "17px"
    }
  },
  markers: {
    size: 0
  },
  xaxis: {
    type: 'datetime',
    range: 20000,
  },
  yaxis: {
    max: 3
  },
  legend: {
    show: true,
    floating: true,
    horizontalAlign: "left",
    onItemClick: {
      toggleDataSeries: false
    },
    position: "top",
    offsetY: -33,
    offsetX: 100
  },
};

var espchart = new ApexCharts(document.querySelector("#espchart"), EspOptions);
espchart.render();
