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
    setInterval(fetchData, 1000);
};


