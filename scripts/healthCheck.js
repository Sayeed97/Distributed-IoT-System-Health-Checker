// All the known states for the server response
const API_RESPONSE_STATE = {
    WAITING: "WAITING",
    SUCCESSFUL: "SUCCESSFUL", // Dummy state, it is present for context purposes
    TIMEOUT: "TIMEOUT",
    ERROR: "ERROR",
    UNKNOWN: "UNKNOWN"
}

/* 
Required information for a host health metrics
Keys is the host IP address
Value is the host Response
*/
let hostHealthMetrics = new Map();
hostHealthMetrics.set("http://192.168.68.107", API_RESPONSE_STATE.WAITING);
hostHealthMetrics.set("http://192.168.68.109", API_RESPONSE_STATE.WAITING);
hostHealthMetrics.set("http://192.168.68.110", API_RESPONSE_STATE.WAITING);

async function performHealthCheck(hostIpAddress) {
    // 4 second timeout for fetch API
    const timeoutSignal = AbortSignal.timeout(4000);
    // If fetch succeeds record the API response else record the API response failure state
    try {
        await fetch(hostIpAddress + "/healthy", { signal: timeoutSignal, method: 'GET',
        headers: {'Accept': 'application/json', },})
        .then(async response => { if (response.ok) {
            const jsonResponse = await response.json();
            hostHealthMetrics.set(hostIpAddress, jsonResponse);
            console.log(jsonResponse);
        } else { hostHealthMetrics.set(hostIpAddress, API_RESPONSE_STATE.ERROR); throw new Error("Invalid Response"); }
    });
    } catch(fetchException) {
        // Log the exception on the console
        console.log(fetchException);
        // Set the API response failure state for each host
        hostHealthMetrics.set(hostIpAddress, fetchException instanceof DOMException ? API_RESPONSE_STATE.TIMEOUT : API_RESPONSE_STATE.UNKNOWN);
    }
}

async function onClickHandler() {
    // Perform a sequential health check on every host
    for (const hostIpAddress of hostHealthMetrics.keys()) {
        performHealthCheck(hostIpAddress);
    }
    // Render the health metrics table
    renderHealthMetricTable();
    // Log the API response for debugging purposes
    console.log(hostHealthMetrics);
}

/*
Checks if the host is actually up or down by checking its response
UP: When fetch API can return a valid response
DOWN: When we get an invalid response or the fetch API has failed
*/
function isDeviceUp(state) {

    const redEl =  document.createElement("button");
    redEl.className = "network-status red"

    const greenEl =  document.createElement("button");
    greenEl.className = "network-status green"

    return (!API_RESPONSE_STATE[state] || state === API_RESPONSE_STATE.ERROR) ? greenEl : redEl;
}

// Health metrics table definition
function renderHealthMetricTable() {
    const tableEl = document.getElementById("host-health-table");
    const tableBodyEl = tableEl.querySelector("tbody");

    // Iterate through fundamental elements in hostHealthMetrics: 1. Host IP Address 2. Host Health API Response
    hostHealthMetrics.forEach((hostHealthMetric, ipAddress)=>{
        const rowEl = document.getElementById(ipAddress) || document.createElement("tr"); 
        rowEl.id = ipAddress;

        const hostIpAddressEl = document.createElement("td");
        const healthEl = document.createElement("td");
        const networkEl = document.createElement("td");

        healthEl.innerHTML = API_RESPONSE_STATE[hostHealthMetric] ? API_RESPONSE_STATE.UNKNOWN : JSON.stringify(hostHealthMetric);
        hostIpAddressEl.innerHTML = ipAddress;
        networkEl.appendChild(isDeviceUp(hostHealthMetric));

        rowEl.replaceChildren(hostIpAddressEl, healthEl, networkEl);

        tableBodyEl.append(rowEl);
    })
}

//setInterval(onClickHandler, 5000)