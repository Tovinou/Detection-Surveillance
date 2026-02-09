#include "html_page.h"
#include <ArduinoJson.h>

SystemState* HtmlPage::systemState = nullptr;
AppConfig* HtmlPage::config = nullptr;

void HtmlPage::setSystemState(SystemState* state) {
    systemState = state;
}

void HtmlPage::setConfig(AppConfig* cfg) {
    config = cfg;
}

String HtmlPage::generateDashboard() {
    if (!systemState || !config) return "<html><body><h1>System Initializing...</h1><p>Please wait.</p></body></html>";

    String html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Surveillance Dashboard</title>
    <style>)rawliteral" + getCSS() + R"rawliteral(</style>
</head>
<body>
    )rawliteral" + getHeader() + R"rawliteral(
    
    <div class="container fade-in">
        <div class="dashboard-grid">
            <!-- Status Card -->
            <div id="status-card" class="card status-card">
                <h2>System Status</h2>
                <div class="status-indicator" id="status-indicator">
                    <span class="pulse"></span>
                    <span id="status-text">Active</span>
                </div>
                <div class="status-details">
                    <p id="connection-status">Checking connection...</p>
                </div>
            </div>

            <!-- Distance Metric -->
            <div class="card metric-card">
                <h3>Distance</h3>
                <div class="metric-value-container">
                    <span class="metric-value" id="distance">--</span>
                    <span class="metric-unit">cm</span>
                </div>
                <div class="progress-bar-bg">
                    <div id="distance-bar" class="progress-bar" style="width: 0%"></div>
                </div>
                <p class="metric-sub">Threshold: )rawliteral" + String(config->system.distance_threshold, 1) + R"rawliteral( cm</p>
            </div>

            <!-- Pi Data -->
            <div class="card info-card">
                <h3>Sensor Data (Pi)</h3>
                <ul class="info-list">
                    <li><span>Mode:</span> <span id="pi-mode">--</span></li>
                    <li><span>Alert:</span> <span id="pi-alert">--</span></li>
                    <li><span>History:</span> <span id="pi-history" style="font-size: 0.85em; font-family: monospace;">--</span></li>
                </ul>
            </div>

            <!-- System Info -->
            <div class="card info-card">
                <h3>System Info</h3>
                <ul class="info-list">
                    <li><span>WiFi Mode:</span> <span>)rawliteral" + systemState->wifiMode + R"rawliteral(</span></li>
                    <li><span>Uptime:</span> <span id="uptime">--</span></li>
                    <li><span>Memory:</span> <span id="memory">--</span></li>
                    <li><span>Timestamp:</span> <span id="timestamp">--</span></li>
                </ul>
            </div>
            
            <!-- Controls -->
            <div class="card controls-card">
                <h3>Controls</h3>
                <div class="button-group">
                    <button onclick="sendCommand('test_alert')" class="btn btn-warning">🔔 Test Alert</button>
                    <button onclick="sendCommand('refresh')" class="btn btn-primary">🔄 Refresh</button>
                    <button onclick="location.reload()" class="btn btn-secondary">🔁 Reload Page</button>
                </div>
            </div>
        </div>
    </div>

    )rawliteral" + getFooter() + R"rawliteral(

    <script>
        const REFRESH_INTERVAL = )rawliteral" + String(config->system.web_refresh_interval * 1000) + R"rawliteral(;
        const threshold = )rawliteral" + String(config->system.distance_threshold) + R"rawliteral(;
        
        function updateDashboard() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // Update Distance
                    document.getElementById('distance').innerText = data.distance.toFixed(1);
                    const distPercent = Math.min((data.distance / 400) * 100, 100);
                    const bar = document.getElementById('distance-bar');
                    bar.style.width = distPercent + '%';
                    
                    if (data.distance < threshold) {
                        bar.style.backgroundColor = '#ff4444';
                    } else {
                        bar.style.backgroundColor = '#00C851';
                    }

                    // Update Status
                    const statusCard = document.getElementById('status-card');
                    const statusText = document.getElementById('status-text');
                    const statusInd = document.getElementById('status-indicator');
                    
                    if (data.object_detected) {
                        statusCard.classList.add('alert-mode');
                        statusText.innerText = "ALERT: Object Detected!";
                        statusInd.classList.add('alert');
                    } else {
                        statusCard.classList.remove('alert-mode');
                        statusText.innerText = data.status;
                        statusInd.classList.remove('alert');
                    }
                    
                    document.getElementById('connection-status').innerText = data.wifi_connected ? "WiFi Connected" : "WiFi Disconnected";

                    // Update Pi Data
                    document.getElementById('pi-mode').innerText = getModeName(data.mode);
                    const alertSpan = document.getElementById('pi-alert');
                    alertSpan.innerText = data.alert_active ? "ACTIVE" : "Normal";
                    alertSpan.style.color = data.alert_active ? '#ff4444' : '#00C851';
                    
                    if(data.pi_history && Array.isArray(data.pi_history)) {
                         document.getElementById('pi-history').innerText = '[' + data.pi_history.map(n => n.toFixed(1)).join(', ') + ']';
                    }

                    // Update Info
                    document.getElementById('uptime').innerText = formatUptime(data.uptime);
                    document.getElementById('memory').innerText = Math.round(data.free_memory / 1024) + ' KB';
                    document.getElementById('timestamp').innerText = new Date(data.timestamp).toLocaleTimeString();
                })
                .catch(err => console.error('Update failed', err));
        }

        function sendCommand(cmd) {
            fetch('/api/command?command=' + cmd, { method: 'POST' })
                .then(res => res.text())
                .then(txt => alert('Command Result: ' + txt))
                .catch(err => alert('Failed: ' + err));
        }
        
        function formatUptime(seconds) {
            const h = Math.floor(seconds / 3600);
            const m = Math.floor((seconds % 3600) / 60);
            const s = seconds % 60;
            return `${h}h ${m}m ${s}s`;
        }

        function getModeName(m) {
            if(m === 0) return "Normal";
            if(m === 1) return "History";
            if(m === 2) return "Directional";
            return "Unknown (" + m + ")";
        }

        setInterval(updateDashboard, REFRESH_INTERVAL);
        document.addEventListener('DOMContentLoaded', updateDashboard);
    </script>
</body>
</html>
)rawliteral";
    return html;
}

String HtmlPage::generateAPIResponse() {
    if (!systemState || !config) return "{}";

    StaticJsonDocument<1024> doc; // Increased size for history array
    doc["distance"] = systemState->currentData.distance;
    doc["object_detected"] = systemState->currentData.object_detected;
    doc["status"] = systemState->currentData.status;
    doc["timestamp"] = systemState->currentData.timestamp;
    
    // New fields from Pi
    doc["mode"] = systemState->currentData.mode;
    doc["alert_active"] = systemState->currentData.alert_active;
    
    JsonArray history = doc.createNestedArray("pi_history");
    for(int i=0; i<5; i++) {
        history.add(systemState->currentData.pi_history[i]);
    }
    
    doc["uptime"] = systemState->systemUptime;
    doc["free_memory"] = ESP.getFreeHeap();
    doc["wifi_connected"] = systemState->wifiConnected;
    doc["wifi_mode"] = systemState->wifiMode;
    doc["threshold"] = config->system.distance_threshold;
    
    String response;
    serializeJson(doc, response);
    return response;
}

String HtmlPage::generateConfigPage() {
    if (!config) return "Config not loaded";
    
    String html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuration</title>
    <style>)rawliteral" + getCSS() + R"rawliteral(</style>
</head>
<body>
    )rawliteral" + getHeader() + R"rawliteral(
    <div class="container fade-in">
        <div class="card">
            <h2>⚙️ System Configuration</h2>
            <form id="configForm">
                <div class="form-group">
                    <label>Distance Threshold (cm)</label>
                    <input type="number" name="threshold" value=")rawliteral" + String(config->system.distance_threshold) + R"rawliteral(" step="0.1" min="1" max="400">
                </div>
                <div class="form-group">
                    <label>Alert Cooldown (seconds)</label>
                    <input type="number" name="cooldown" value=")rawliteral" + String(config->system.alert_cooldown / 1000) + R"rawliteral(" min="1">
                </div>
                <div class="form-group">
                    <label>Telegram Alerts</label>
                    <select name="telegram">
                        <option value="true" )rawliteral" + (config->telegram.enable_telegram ? "selected" : "") + R"rawliteral(>Enabled</option>
                        <option value="false" )rawliteral" + (!config->telegram.enable_telegram ? "selected" : "") + R"rawliteral(>Disabled</option>
                    </select>
                </div>
                <div class="form-actions">
                    <button type="submit" class="btn btn-primary">Save Changes</button>
                    <button type="button" onclick="location.href='/'" class="btn btn-secondary">Cancel</button>
                </div>
            </form>
        </div>
    </div>
    <script>
        document.getElementById('configForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const formData = new FormData(this);
            const data = Object.fromEntries(formData);
            fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(data)
            }).then(res => res.json()).then(res => {
                alert('Saved!');
                location.href = '/';
            }).catch(err => alert('Error: ' + err));
        });
    </script>
    )rawliteral" + getFooter() + R"rawliteral(
</body>
</html>
)rawliteral";
    return html;
}

String HtmlPage::generateHistoryPage() {
     if (!systemState) return "History unavailable";

    String html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>History Data</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>)rawliteral" + getCSS() + R"rawliteral(</style>
</head>
<body>
    )rawliteral" + getHeader() + R"rawliteral(
    <div class="container fade-in">
        <div class="card">
            <h2>📈 Distance History</h2>
            <div style="position: relative; height:300px; width:100%">
                <canvas id="historyChart"></canvas>
            </div>
        </div>
        <div class="card" style="margin-top: 20px;">
             <h3>Recent Events</h3>
             <table class="data-table">
                <thead><tr><th>Time</th><th>Distance</th><th>Status</th></tr></thead>
                <tbody id="historyTableBody"></tbody>
             </table>
        </div>
    </div>
    <script>
        function loadHistory() {
            fetch('/api/history').then(r => r.json()).then(data => {
                const history = data.history;
                const labels = history.map((_, i) => i); // Simple index or timestamp
                const values = history.map(h => h.distance);
                
                // Update Chart
                new Chart(document.getElementById('historyChart'), {
                    type: 'line',
                    data: {
                        labels: labels,
                        datasets: [{
                            label: 'Distance (cm)',
                            data: values,
                            borderColor: '#33b5e5',
                            tension: 0.4
                        }]
                    },
                    options: { responsive: true, maintainAspectRatio: false }
                });

                // Update Table
                const tbody = document.getElementById('historyTableBody');
                tbody.innerHTML = history.slice(0, 10).map(h => `
                    <tr>
                        <td>${h.timestamp}</td>
                        <td>${h.distance.toFixed(1)} cm</td>
                        <td>${h.object_detected ? '🚨' : '✅'}</td>
                    </tr>
                `).join('');
            });
        }
        loadHistory();
    </script>
    )rawliteral" + getFooter() + R"rawliteral(
</body>
</html>
)rawliteral";
    return html;
}

String HtmlPage::generateErrorPage(const String& message) {
    return R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>Error</title><style>)rawliteral" + getCSS() + R"rawliteral(</style></head>
<body>
    )rawliteral" + getHeader() + R"rawliteral(
    <div class="container">
        <div class="card" style="text-align:center; color: #ff4444;">
            <h1>⚠️ Error</h1>
            <p>)rawliteral" + message + R"rawliteral(</p>
            <button onclick="location.href='/'" class="btn btn-primary">Go Home</button>
        </div>
    </div>
    )rawliteral" + getFooter() + R"rawliteral(
</body>
</html>
)rawliteral";
}

String HtmlPage::getHeader() {
    return R"rawliteral(
    <header class="main-header">
        <div class="logo">🛡️ ESP32 Surveillance</div>
        <nav>
            <a href="/">Dashboard</a>
            <a href="/history">History</a>
            <a href="/config">Settings</a>
        </nav>
    </header>
    )rawliteral";
}

String HtmlPage::getFooter() {
    return R"rawliteral(
    <footer class="main-footer">
        <p>ESP32 Surveillance System v3.0 | Powered by PlatformIO</p>
    </footer>
    )rawliteral";
}

String HtmlPage::formatTimestamp(unsigned long timestamp) {
    // Simple helper
    return String(timestamp);
}

String HtmlPage::getCSS() {
    return R"rawliteral(
        :root {
            --bg-color: #1a1a2e;
            --card-bg: #16213e;
            --text-color: #e94560;
            --text-light: #f1f1f1;
            --accent: #0f3460;
            --success: #00C851;
            --danger: #ff4444;
            --warning: #ffbb33;
        }
        body {
            font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-light);
            margin: 0;
            padding: 0;
            line-height: 1.6;
        }
        .container {
            max-width: 1000px;
            margin: 0 auto;
            padding: 20px;
        }
        .main-header {
            background-color: var(--accent);
            padding: 1rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        }
        .logo { font-size: 1.5rem; font-weight: bold; color: var(--text-color); }
        nav a {
            color: var(--text-light);
            text-decoration: none;
            margin-left: 20px;
            font-weight: 500;
            transition: color 0.3s;
        }
        nav a:hover { color: var(--text-color); }
        
        .dashboard-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .card {
            background-color: var(--card-bg);
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.2s;
        }
        .card:hover { transform: translateY(-2px); }
        .card h2, .card h3 { margin-top: 0; color: var(--text-color); }
        
        .metric-value-container { display: flex; align-items: baseline; }
        .metric-value { font-size: 3rem; font-weight: bold; }
        .metric-unit { font-size: 1.2rem; margin-left: 5px; opacity: 0.7; }
        .metric-sub { font-size: 0.9rem; opacity: 0.6; margin-top: 5px; }
        
        .status-indicator {
            display: flex;
            align-items: center;
            margin: 20px 0;
            font-size: 1.2rem;
        }
        .status-indicator.alert { color: var(--danger); }
        .pulse {
            width: 12px; height: 12px;
            background-color: var(--success);
            border-radius: 50%;
            margin-right: 10px;
            box-shadow: 0 0 0 rgba(0, 200, 81, 0.4);
            animation: pulse 2s infinite;
        }
        .alert .pulse { background-color: var(--danger); box-shadow: 0 0 0 rgba(255, 68, 68, 0.4); }
        
        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(0, 200, 81, 0.4); }
            70% { box-shadow: 0 0 0 10px rgba(0, 200, 81, 0); }
            100% { box-shadow: 0 0 0 0 rgba(0, 200, 81, 0); }
        }
        
        .info-list { list-style: none; padding: 0; }
        .info-list li {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid rgba(255,255,255,0.05);
        }
        
        .button-group { display: flex; flex-direction: column; gap: 10px; }
        .btn {
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            cursor: pointer;
            font-weight: 600;
            transition: opacity 0.2s;
            width: 100%;
        }
        .btn:hover { opacity: 0.9; }
        .btn-primary { background-color: var(--text-color); color: white; }
        .btn-secondary { background-color: #4a5568; color: white; }
        .btn-warning { background-color: var(--warning); color: #333; }
        
        .progress-bar-bg {
            background-color: rgba(255,255,255,0.1);
            height: 10px;
            border-radius: 5px;
            overflow: hidden;
            margin: 10px 0;
        }
        .progress-bar {
            height: 100%;
            background-color: var(--success);
            transition: width 0.5s ease, background-color 0.5s;
        }
        
        .form-group { margin-bottom: 15px; }
        .form-group label { display: block; margin-bottom: 5px; }
        .form-group input, .form-group select {
            width: 100%;
            padding: 8px;
            border-radius: 4px;
            border: 1px solid #444;
            background: #2a2a40;
            color: white;
        }
        
        .data-table { width: 100%; border-collapse: collapse; }
        .data-table th, .data-table td { padding: 10px; text-align: left; border-bottom: 1px solid #333; }
        
        .main-footer {
            text-align: center;
            padding: 20px;
            opacity: 0.5;
            font-size: 0.9rem;
            margin-top: 40px;
        }
        
        .fade-in { animation: fadeIn 0.5s ease-in; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(10px); } to { opacity: 1; transform: translateY(0); } }
        
        /* Alert Mode Style */
        .status-card.alert-mode {
            border: 2px solid var(--danger);
            background: rgba(255, 68, 68, 0.1);
        }
    )rawliteral";
}
