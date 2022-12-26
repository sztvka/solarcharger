var a = [2, 3, 4, 5];
a.forEach(element => {
    console.log(element*3);    
});
var cnt = 0;
const ctx = document.getElementById('myChart');

var voltage = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [1],
    datasets: [{
      label: 'Value',
      data: [0],
      borderWidth: 1
    }]
  },
  options: {
    scales: {
      y: {
        beginAtZero: true
      }
    }
  }
});


updateGauges = (obj) =>{
    cnt++;
    if(voltage.data.labels.length>19){
        voltage.data.labels = [];
        voltage.data.datasets[0].data = [];
        cnt = 1;
    };
    
    voltage.data.labels.push(cnt);
    voltage.data.datasets[0].data.push(obj.ina1.voltage);
    voltage.update();
};





fetchData = () => {
    fetch('data', {
        method: 'GET',
        headers: {
            'Accept': 'application/json',
        },
    })
    .then(response => response.json())
    .then(text => {
        updateGauges(text);

    })
};

window.onload = function() {
    document.getElementsByClassName("mask")[0].style = "visibility: hidden";;
  //  setInterval(fetchData, 1000);
};


function getNewData(baseval, yrange) {
  var newTime = baseval+3600000;
  return {
    x: newTime,
    y: Math.floor(Math.random() * (yrange.max - yrange.min + 1)) + yrange.min
  };
}

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
}



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
    max: 40
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




window.setInterval(function () {
  var b = {};
  var a = {};
  b = getNewData(Date.now(), {min: 0, max:30});
  a = getNewData(Date.now(), {min: 0, max:20});
  solardata.push([a.x,a.y]);
  espdata.push([b.x,b.y]);
  espchart.updateSeries([{
    data: espdata
  },
  {
    data: solardata
  }]);
}, 1000)

