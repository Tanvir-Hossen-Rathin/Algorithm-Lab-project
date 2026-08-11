let devices = JSON.parse(localStorage.getItem('energyDevices')) || [
    { id: 1, name: "AC (1.5 Ton)", room: "Bedroom", watts: 1800, hours: 8 },
    { id: 2, name: "Refrigerator", room: "Kitchen", watts: 200, hours: 24 }
];

const ANOMALY_THRESHOLD = 5.0; // kWh

function updateAll() {
    renderTable();
    calculateTotals();
    checkAnomalies();
    generateRecs();
    localStorage.setItem('energyDevices', JSON.stringify(devices));
}

function calculateTotals() {
    const rate = parseFloat(document.getElementById('tariffRate').value);
    let totalKwh = 0;

    devices.forEach(d => {
        totalKwh += (d.watts * d.hours) / 1000;
    });

    const dailyCost = totalKwh * rate;
    
    document.getElementById('totalKwh').innerText = `${totalKwh.toFixed(2)} kWh`;
    document.getElementById('totalCost').innerText = `$${dailyCost.toFixed(2)}`;
    document.getElementById('monthlyCost').innerText = `$${(dailyCost * 30).toFixed(2)}`;
}

function renderTable() {
    const tbody = document.getElementById('deviceTableBody');
    const rate = parseFloat(document.getElementById('tariffRate').value);
    tbody.innerHTML = '';

    devices.forEach(d => {
        const kwh = (d.watts * d.hours) / 1000;
        const cost = kwh * rate;
        
        const row = `
            <tr>
                <td>${d.name}</td>
                <td>${d.room}</td>
                <td>${d.watts}W</td>
                <td>${d.hours}</td>
                <td>${kwh.toFixed(2)}</td>
                <td>$${cost.toFixed(2)}</td>
                <td><button class="btn-delete" onclick="removeDevice(${d.id})"><i class="fas fa-trash"></i></button></td>
            </tr>
        `;
        tbody.innerHTML += row;
    });
}

document.getElementById('deviceForm').addEventListener('submit', (e) => {
    e.preventDefault();
    const newDevice = {
        id: Date.now(),
        name: document.getElementById('deviceName').value,
        room: document.getElementById('roomName').value,
        watts: parseFloat(document.getElementById('wattage').value),
        hours: parseFloat(document.getElementById('hours').value)
    };
    devices.push(newDevice);
    e.target.reset();
    updateAll();
});

function removeDevice(id) {
    devices = devices.filter(d => d.id !== id);
    updateAll();
}

function sortDevices() {
    // QuickSort logic simplified for JS
    devices.sort((a, b) => ((b.watts * b.hours) - (a.watts * a.hours)));
    updateAll();
}

function checkAnomalies() {
    const list = document.getElementById('anomalyList');
    list.innerHTML = '';
    let flagged = false;

    devices.forEach(d => {
        const kwh = (d.watts * d.hours) / 1000;
        if (kwh > ANOMALY_THRESHOLD) {
            list.innerHTML += `<li class="alert-item"><i class="fas fa-exclamation-circle"></i> ${d.name} usage is high (${kwh.toFixed(1)} kWh)</li>`;
            flagged = true;
        }
    });

    if (!flagged) list.innerHTML = '<li class="safe">System nominal. No spikes detected.</li>';
}

function generateRecs() {
    const recList = document.getElementById('recList');
    recList.innerHTML = '';
    
    if (devices.length === 0) {
        recList.innerHTML = '<li>Add devices to see personalized recommendations.</li>';
        return;
    }

    devices.forEach(d => {
        if (d.hours > 12 && d.watts > 100) {
            recList.innerHTML += `<li><b>${d.name}:</b> High runtime. Reducing use by 2 hours saves ~$${((d.watts * 2 / 1000) * 0.15 * 30).toFixed(2)}/mo.</li>`;
        }
        if (d.watts >= 1500) {
            recList.innerHTML += `<li><b>${d.name}:</b> Heavy load detected. Use during off-peak hours if possible.</li>`;
        }
    });
}

// Initial Load
updateAll();