#pragma once

const char* web_ui_html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>机械臂控制台</title>
    <style>
        :root {
            --bg-gradient: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
            --card-bg: rgba(30, 32, 48, 0.95);
            --card-border: rgba(99, 102, 241, 0.25);
            --primary: #818cf8;
            --primary-glow: rgba(129, 140, 248, 0.4);
            --accent: #f472b6;
            --accent-glow: rgba(244, 114, 182, 0.3);
            --success: #34d399;
            --danger: #f87171;
            --warning: #fbbf24;
            --text-primary: #e2e8f0;
            --text-secondary: #94a3b8;
            --glass-bg: rgba(255,255,255,0.06);
            --glass-border: rgba(255,255,255,0.1);
            --radius: 16px;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif;
            background: var(--bg-gradient);
            min-height: 100vh;
            display: flex; flex-direction: column; align-items: center;
            padding: 12px 8px 40px;
            color: var(--text-primary);
        }

        /* ===== 顶部状态栏 ===== */
        .status-bar {
            width: 100%; max-width: 720px;
            display: flex; justify-content: space-between; align-items: center;
            padding: 10px 16px;
            background: var(--glass-bg);
            border: 1px solid var(--glass-border);
            border-radius: var(--radius);
            backdrop-filter: blur(12px);
            margin-bottom: 14px;
            font-size: 13px;
        }
        .status-left { display: flex; align-items: center; gap: 6px; }
        .status-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--success); box-shadow: 0 0 6px var(--success); }
        .status-dot.offline { background: var(--danger); box-shadow: 0 0 6px var(--danger); }
        .gamepad-badge {
            display: flex; align-items: center; gap: 5px; cursor: pointer;
            padding: 4px 10px; border-radius: 20px;
            background: linear-gradient(135deg, rgba(129,140,248,0.2), rgba(244,114,182,0.2));
            border: 1px solid rgba(129,140,248,0.3);
            transition: all 0.2s;
            font-size: 12px; font-weight: 600;
        }
        .gamepad-badge:hover { border-color: var(--primary); background: rgba(129,140,248,0.3); }
        .gamepad-badge.connected { background: linear-gradient(135deg, rgba(52,211,153,0.2), rgba(129,140,248,0.2)); border-color: var(--success); }
        .wifi-info { display: flex; gap: 12px; font-size: 11px; color: var(--text-secondary); }

        /* ===== 标题 ===== */
        .title-area { text-align: center; margin-bottom: 16px; }
        .title-area h1 {
            font-size: 22px; font-weight: 700;
            background: linear-gradient(90deg, var(--primary), var(--accent));
            -webkit-background-clip: text; -webkit-text-fill-color: transparent;
        }
        .title-area p { font-size: 12px; color: var(--text-secondary); margin-top: 2px; }

        /* ===== 控制卡片 ===== */
        .cards { display: grid; grid-template-columns: repeat(2, 1fr); gap: 12px; width: 100%; max-width: 720px; }
        @media (min-width: 600px) { .cards { grid-template-columns: repeat(4, 1fr); } }

        .card {
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: var(--radius);
            overflow: hidden;
            transition: transform 0.15s, box-shadow 0.15s;
        }
        .card:hover { transform: translateY(-2px); box-shadow: 0 8px 24px rgba(0,0,0,0.3); }

        .card-title {
            padding: 10px 12px; text-align: center;
            font-size: 13px; font-weight: 700; letter-spacing: 0.5px;
            background: linear-gradient(135deg, rgba(129,140,248,0.15), rgba(244,114,182,0.1));
            color: var(--primary);
        }

        .ctrl-pad {
            position: relative; height: 110px; margin: 10px;
            border-radius: 12px;
            background: linear-gradient(145deg, rgba(255,255,255,0.03), rgba(0,0,0,0.15));
            border: 1px solid var(--glass-border);
            overflow: hidden;
            display: flex;
        }
        .ctrl-pad::before {
            content: ''; position: absolute; top: 0; bottom: 0;
            left: 50%; width: 1px;
            background: linear-gradient(to bottom, transparent, var(--glass-border), transparent);
            z-index: 1;
        }

        .btn-half {
            flex: 1; display: flex; align-items: center; justify-content: center;
            font-size: 28px; font-weight: 800; cursor: pointer; user-select: none;
            color: rgba(255,255,255,0.25); transition: all 0.1s; z-index: 2;
            -webkit-tap-highlight-color: transparent;
        }
        .btn-half:active, .btn-half.active { background: var(--primary-glow); color: var(--primary); }

        .btn-center {
            position: absolute; bottom: 8px; left: 50%; transform: translateX(-50%);
            z-index: 3; width: 28px; height: 28px; border-radius: 50%;
            display: flex; align-items: center; justify-content: center; cursor: pointer;
            transition: all 0.15s;
        }
        .btn-center.stop-btn { border: 2px solid var(--danger); background: rgba(248,113,113,0.1); }
        .btn-center.stop-btn:active { background: var(--danger); }
        .btn-center.stop-btn .icon { width: 8px; height: 8px; background: var(--danger); border-radius: 1px; }
        .btn-center.stop-btn:active .icon { background: white; }
        .btn-center.center-btn { border: 2px solid var(--warning); background: rgba(251,191,36,0.1); }
        .btn-center.center-btn:active { background: var(--warning); }
        .btn-center.center-btn .icon { width: 8px; height: 8px; background: var(--warning); border-radius: 50%; }
        .btn-center.center-btn:active .icon { background: white; }

        .card-status {
            padding: 8px 12px; text-align: center;
            font-size: 13px; font-weight: 700;
            border-top: 1px solid rgba(255,255,255,0.05);
            color: var(--text-secondary); cursor: pointer; transition: color 0.15s;
        }
        .card-status:active { color: var(--primary); }
        .card-status .angle-val { color: var(--warning); font-size: 16px; }

        /* ===== 折叠面板通用 ===== */
        .panel {
            width: 100%; max-width: 720px; margin-top: 14px;
            background: var(--card-bg); border: 1px solid var(--card-border);
            border-radius: var(--radius); overflow: hidden;
        }
        .panel-toggle {
            padding: 12px 16px; cursor: pointer;
            display: flex; align-items: center; justify-content: space-between;
            font-size: 14px; font-weight: 700;
        }
        .panel-toggle:hover { opacity: 0.85; }
        .panel-body { display: none; padding: 16px; }
        .panel-body.open { display: block; }

        .panel-toggle.t-primary { color: var(--primary); background: linear-gradient(135deg, rgba(129,140,248,0.1), rgba(244,114,182,0.05)); }
        .panel-toggle.t-accent { color: var(--accent); background: linear-gradient(135deg, rgba(244,114,182,0.1), rgba(129,140,248,0.05)); }
        .panel-toggle.t-success { color: var(--success); background: linear-gradient(135deg, rgba(52,211,153,0.1), rgba(129,140,248,0.05)); }

        /* ===== 高级设置 grid ===== */
        .adv-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 14px; }
        @media (max-width: 550px) { .adv-grid { grid-template-columns: 1fr; } }

        .adv-card {
            background: var(--glass-bg); border: 1px solid var(--glass-border);
            border-radius: 12px; padding: 14px;
        }
        .adv-card h4 {
            font-size: 13px; color: var(--primary); margin-bottom: 12px;
            padding-bottom: 6px; border-bottom: 1px solid var(--glass-border);
        }
        .param-row { margin-bottom: 12px; }
        .param-label {
            display: flex; justify-content: space-between; align-items: center;
            font-size: 12px; color: var(--text-secondary); margin-bottom: 4px;
        }
        .param-val { color: var(--primary); font-weight: 700; font-size: 13px; }

        input[type=range] {
            width: 100%; -webkit-appearance: none; background: rgba(255,255,255,0.08);
            height: 4px; border-radius: 2px; outline: none; margin: 6px 0;
        }
        input[type=range]::-webkit-slider-thumb {
            -webkit-appearance: none; width: 14px; height: 14px; border-radius: 50%;
            background: var(--primary); cursor: pointer; box-shadow: 0 0 6px var(--primary-glow);
        }
        .range-hint { display: flex; justify-content: space-between; font-size: 10px; color: rgba(255,255,255,0.25); }

        /* ===== 映射设置 ===== */
        .map-grid { display: grid; grid-template-columns: 1fr; gap: 10px; }
        .map-row {
            display: grid; grid-template-columns: 100px 1fr 60px; gap: 8px; align-items: center;
            background: var(--glass-bg); border: 1px solid var(--glass-border);
            border-radius: 10px; padding: 10px 12px;
        }
        .map-label { font-size: 12px; font-weight: 700; color: var(--primary); }
        .map-input {
            background: rgba(0,0,0,0.3); border: 1px solid var(--glass-border);
            border-radius: 6px; padding: 6px 8px; color: var(--text-primary);
            font-size: 12px; font-family: 'Consolas', monospace; width: 100%;
            outline: none; transition: border-color 0.2s;
        }
        .map-input:focus { border-color: var(--primary); }
        select.map-input { cursor: pointer; }
        .map-hint { font-size: 10px; color: var(--text-secondary); }
        .map-section-title {
            font-size: 13px; font-weight: 700; color: var(--accent);
            margin: 10px 0 6px; padding-bottom: 4px;
            border-bottom: 1px solid rgba(244,114,182,0.2);
        }
        .map-section-title:first-child { margin-top: 0; }
        .map-live {
            display: inline-block; padding: 2px 8px; border-radius: 10px; font-size: 11px;
            background: rgba(52,211,153,0.15); color: var(--success); font-weight: 700;
            min-width: 50px; text-align: center;
        }
        .map-actions { display: flex; gap: 8px; margin-top: 12px; }
        .map-btn {
            flex: 1; padding: 8px; border: none; border-radius: 8px;
            font-size: 12px; font-weight: 700; cursor: pointer; transition: opacity 0.15s;
        }
        .map-btn:hover { opacity: 0.85; }
        .map-btn.save { background: linear-gradient(135deg, var(--primary), var(--accent)); color: white; }
        .map-btn.reset { background: var(--glass-bg); border: 1px solid var(--glass-border); color: var(--text-secondary); }

        /* ===== 配对弹窗 ===== */
        .modal-overlay {
            display: none; position: fixed; inset: 0; background: rgba(0,0,0,0.6);
            z-index: 1000; justify-content: center; align-items: center;
            backdrop-filter: blur(4px);
        }
        .modal-overlay.open { display: flex; }
        .modal-box {
            background: var(--card-bg); border: 1px solid var(--card-border);
            border-radius: var(--radius); padding: 24px; width: 90%; max-width: 380px;
            box-shadow: 0 16px 48px rgba(0,0,0,0.4);
        }
        .modal-box h3 { color: var(--primary); margin-bottom: 14px; font-size: 18px; }
        .modal-box ol { margin: 12px 0 12px 20px; line-height: 1.8; font-size: 13px; color: var(--text-secondary); }
        .modal-box ol b { color: var(--text-primary); }
        .modal-box .warn { color: var(--danger); font-size: 12px; margin-top: 8px; }
        .modal-btn {
            margin-top: 16px; width: 100%; padding: 10px; border: none; border-radius: 10px;
            font-size: 14px; font-weight: 700; cursor: pointer; color: white;
            background: linear-gradient(135deg, var(--primary), var(--accent));
        }
        .modal-btn:hover { opacity: 0.9; }

        /* ===== 调试 ===== */
        .debug-log {
            font-family: 'Consolas','Monaco',monospace; font-size: 11px;
            color: var(--success); line-height: 1.6;
            max-height: 180px; overflow-y: auto;
        }
    </style>
</head>
<body>

    <!-- 状态栏 -->
    <div class="status-bar">
        <div class="status-left">
            <div class="status-dot" id="conn-dot"></div>
            <span id="conn-status">在线</span>
        </div>
        <div class="gamepad-badge" id="gp-badge" onclick="showModal()">
            <span id="gamepad-status">🎮 未连接</span>
        </div>
        <div class="wifi-info">
            <span id="wifi-rssi">📶 --</span>
            <span id="wifi-uptime">⏱ 0s</span>
        </div>
    </div>

    <!-- 标题 -->
    <div class="title-area">
        <h1>🦾 机械臂控制台</h1>
        <p>ESP32-C3 · WiFi + Gamepad 双模控制</p>
    </div>

    <!-- 配对弹窗 -->
    <div class="modal-overlay" id="pair-modal">
        <div class="modal-box">
            <h3>🎮 手柄配对指南</h3>
            <ol>
                <li>打开<b>电脑/手机</b>的蓝牙设置</li>
                <li>将手柄开机并进入配对模式<br>(小鸡: 长按 <b>A+Home</b> 或 <b>B+Home</b> 直到快闪)</li>
                <li>在蓝牙列表中点击<b>连接</b></li>
                <li>回到本页面，<b>按下手柄任意键</b>激活</li>
            </ol>
            <p class="warn">⚠ 连接后若无反应，请刷新页面并重按一下手柄按键</p>
            <button class="modal-btn" onclick="hideModal()">我知道了</button>
        </div>
    </div>

    <!-- 控制卡片 -->
    <div class="cards">
        <div class="card">
            <div class="card-title">⚙ TT电机 底座</div>
            <div class="ctrl-pad">
                <div class="btn-half" onmousedown="webDown('tt','up')" onmouseup="webUp('tt')" ontouchstart="webDown('tt','up')" ontouchend="webUp('tt')">▲</div>
                <div class="btn-half" onmousedown="webDown('tt','down')" onmouseup="webUp('tt')" ontouchstart="webDown('tt','down')" ontouchend="webUp('tt')">▼</div>
                <div class="btn-center stop-btn" onclick="sendApi('tt','stop')"><div class="icon"></div></div>
            </div>
            <div class="card-status" onclick="sendApi('tt','stop')"><span id="tt-state">状态: 停止</span></div>
        </div>
        <div class="card">
            <div class="card-title">🔄 舵机1 大臂</div>
            <div class="ctrl-pad">
                <div class="btn-half" onmousedown="webDown('servo1','left')" onmouseup="webUp('servo1')" ontouchstart="webDown('servo1','left')" ontouchend="webUp('servo1')">◀</div>
                <div class="btn-half" onmousedown="webDown('servo1','right')" onmouseup="webUp('servo1')" ontouchstart="webDown('servo1','right')" ontouchend="webUp('servo1')">▶</div>
                <div class="btn-center stop-btn" onclick="sendApi('servo1','stop')"><div class="icon"></div></div>
            </div>
            <div class="card-status" onclick="sendApi('servo1','stop')"><span id="s1-state">状态: 停止</span></div>
        </div>
        <div class="card">
            <div class="card-title">🔄 舵机2 小臂</div>
            <div class="ctrl-pad">
                <div class="btn-half" onmousedown="webDown('servo2','left')" onmouseup="webUp('servo2')" ontouchstart="webDown('servo2','left')" ontouchend="webUp('servo2')">◀</div>
                <div class="btn-half" onmousedown="webDown('servo2','right')" onmouseup="webUp('servo2')" ontouchstart="webDown('servo2','right')" ontouchend="webUp('servo2')">▶</div>
                <div class="btn-center stop-btn" onclick="sendApi('servo2','stop')"><div class="icon"></div></div>
            </div>
            <div class="card-status" onclick="sendApi('servo2','stop')"><span id="s2-state">状态: 停止</span></div>
        </div>
        <div class="card">
            <div class="card-title">✋ 舵机3 夹爪</div>
            <div class="ctrl-pad">
                <div class="btn-half" onmousedown="webDown('servo3','left')" onmouseup="webUp('servo3')" ontouchstart="webDown('servo3','left')" ontouchend="webUp('servo3')">◁</div>
                <div class="btn-half" onmousedown="webDown('servo3','right')" onmouseup="webUp('servo3')" ontouchstart="webDown('servo3','right')" ontouchend="webUp('servo3')">▷</div>
                <div class="btn-center center-btn" onclick="sendApi('servo3','center')"><div class="icon"></div></div>
            </div>
            <div class="card-status" onclick="sendApi('servo3','center')"><span id="s3-state">状态: 静止</span> | 角度: <span class="angle-val" id="servo3-angle-display">90°</span></div>
        </div>
    </div>

    <!-- 高级设置 -->
    <div class="panel">
        <div class="panel-toggle t-primary" onclick="togglePanel('adv')">
            <span>⚙ 高级设置</span><span id="adv-arrow">▼</span>
        </div>
        <div class="panel-body" id="adv-body">
            <div class="adv-grid">
                <div class="adv-card">
                    <h4>TT电机</h4>
                    <div class="param-row">
                        <div class="param-label"><span>速度</span><span class="param-val" id="tt-speed-val">255</span></div>
                        <input type="range" id="tt-speed" min="80" max="255" value="255" oninput="updVal('tt-speed')">
                    </div>
                </div>
                <div class="adv-card">
                    <h4>舵机1 大臂</h4>
                    <div class="param-row">
                        <div class="param-label"><span>速度</span><span class="param-val" id="s1-speed-val">255</span></div>
                        <input type="range" id="s1-speed" min="80" max="255" value="255" oninput="updVal('s1-speed')">
                    </div>
                </div>
                <div class="adv-card">
                    <h4>舵机2 小臂</h4>
                    <div class="param-row">
                        <div class="param-label"><span>速度</span><span class="param-val" id="s2-speed-val">255</span></div>
                        <input type="range" id="s2-speed" min="80" max="255" value="255" oninput="updVal('s2-speed')">
                    </div>
                </div>
                <div class="adv-card">
                    <h4>舵机3 夹爪 (180°)</h4>
                    <div class="param-row">
                        <div class="param-label"><span>移动速度</span><span class="param-val" id="s3-speed-val">180</span></div>
                        <input type="range" id="s3-speed" min="50" max="255" value="180" oninput="updVal('s3-speed')">
                        <div class="range-hint"><span>慢</span><span>快</span></div>
                    </div>
                    <div class="param-row">
                        <div class="param-label"><span>最大角度限制</span><span class="param-val" id="s3-max-val">160°</span></div>
                        <input type="range" id="s3-max" min="0" max="180" value="160" oninput="updMax('s3-max')" onchange="sendMaxAngle()">
                        <div class="range-hint"><span>0°</span><span>180°</span></div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <!-- 🎮 手柄映射设置 -->
    <div class="panel">
        <div class="panel-toggle t-success" onclick="togglePanel('map')">
            <span>🎮 手柄按键映射</span><span id="map-arrow">▼</span>
        </div>
        <div class="panel-body" id="map-body">
            <!-- 实时检测 -->
            <div style="background:rgba(52,211,153,0.08); border:1px solid rgba(52,211,153,0.2); border-radius:10px; padding:10px 14px; margin-bottom:14px;">
                <div style="font-size:12px; font-weight:700; color:var(--success); margin-bottom:6px;">📡 实时检测 (按下手柄按键/摇杆即可看到编号)</div>
                <div style="display:flex; gap:16px; font-size:11px; font-family:Consolas,monospace;">
                    <div>轴: <span id="live-axes" class="map-live">--</span></div>
                    <div>按钮: <span id="live-btns" class="map-live">--</span></div>
                </div>
            </div>

            <!-- 摇杆映射 -->
            <div class="map-section-title">🕹 摇杆映射 (轴编号)</div>
            <div class="map-grid">
                <div class="map-row">
                    <div class="map-label">TT电机 底座</div>
                    <select class="map-input" id="map-tt-axis"><option value="0">轴0 (左摇杆X)</option><option value="1">轴1 (左摇杆Y)</option><option value="2">轴2 (右摇杆X)</option><option value="3">轴3 (右摇杆Y)</option><option value="4">轴4</option><option value="5">轴5</option></select>
                    <label style="font-size:11px;color:var(--text-secondary);display:flex;align-items:center;gap:4px;"><input type="checkbox" id="map-tt-inv"> 反转</label>
                </div>
                <div class="map-row">
                    <div class="map-label">舵机1 大臂</div>
                    <select class="map-input" id="map-s1-axis"><option value="0">轴0 (左摇杆X)</option><option value="1" selected>轴1 (左摇杆Y)</option><option value="2">轴2 (右摇杆X)</option><option value="3">轴3 (右摇杆Y)</option><option value="4">轴4</option><option value="5">轴5</option></select>
                    <label style="font-size:11px;color:var(--text-secondary);display:flex;align-items:center;gap:4px;"><input type="checkbox" id="map-s1-inv"> 反转</label>
                </div>
                <div class="map-row">
                    <div class="map-label">舵机2 小臂</div>
                    <select class="map-input" id="map-s2-axis"><option value="0">轴0 (左摇杆X)</option><option value="1">轴1 (左摇杆Y)</option><option value="2">轴2 (右摇杆X)</option><option value="3" selected>轴3 (右摇杆Y)</option><option value="4">轴4</option><option value="5">轴5</option></select>
                    <label style="font-size:11px;color:var(--text-secondary);display:flex;align-items:center;gap:4px;"><input type="checkbox" id="map-s2-inv"> 反转</label>
                </div>
            </div>

            <!-- 按钮映射 -->
            <div class="map-section-title">🔘 按钮映射 (按钮编号，多个用逗号分隔)</div>
            <div class="map-grid">
                <div class="map-row">
                    <div class="map-label">夹爪 张开</div>
                    <div style="display:flex; gap:6px; flex:1;">
                        <input class="map-input" id="map-s3-open" type="text" value="4,6" placeholder="可手动修改">
                        <div onclick="startBind('map-s3-open')" style="cursor:pointer; background:rgba(52,211,153,0.15); border-radius:4px; display:flex; align-items:center; justify-content:center; width:36px; font-weight:bold; color:var(--success);" title="点击后按手柄追加按键">＋</div>
                    </div>
                    <div class="map-hint">L1/L2等</div>
                </div>
                <div class="map-row">
                    <div class="map-label">夹爪 闭合</div>
                    <div style="display:flex; gap:6px; flex:1;">
                        <input class="map-input" id="map-s3-close" type="text" value="5,7" placeholder="可手动修改">
                        <div onclick="startBind('map-s3-close')" style="cursor:pointer; background:rgba(52,211,153,0.15); border-radius:4px; display:flex; align-items:center; justify-content:center; width:36px; font-weight:bold; color:var(--success);" title="点击后按手柄追加按键">＋</div>
                    </div>
                    <div class="map-hint">R1/R2等</div>
                </div>
                <div class="map-row">
                    <div class="map-label">夹爪 归中</div>
                    <div style="display:flex; gap:6px; flex:1;">
                        <input class="map-input" id="map-s3-center" type="text" value="1" placeholder="可手动修改">
                        <div onclick="startBind('map-s3-center')" style="cursor:pointer; background:rgba(52,211,153,0.15); border-radius:4px; display:flex; align-items:center; justify-content:center; width:36px; font-weight:bold; color:var(--success);" title="点击后按手柄追加按键">＋</div>
                    </div>
                    <div class="map-hint">B键等</div>
                </div>
            </div>

            <!-- 死区 -->
            <div class="map-section-title">⚙ 其他</div>
            <div class="map-row">
                <div class="map-label">摇杆死区</div>
                <input class="map-input" id="map-deadzone" type="number" step="0.05" min="0.05" max="0.8" value="0.25" style="width:80px;">
                <div class="map-hint">0.05-0.8</div>
            </div>

            <div class="map-actions">
                <button class="map-btn save" onclick="saveMapping()">💾 保存映射</button>
                <button class="map-btn reset" onclick="resetMapping()">↩ 恢复默认</button>
            </div>
        </div>
    </div>

    <!-- 手柄调试 -->
    <div class="panel">
        <div class="panel-toggle t-accent" onclick="togglePanel('debug')">
            <span>🐛 手柄调试</span><span id="debug-arrow">▼</span>
        </div>
        <div class="panel-body" id="debug-body">
            <div class="debug-log" id="debug-log">等待手柄连接...</div>
        </div>
    </div>

<script>
// ===== 工具函数 =====
function showModal() { document.getElementById('pair-modal').classList.add('open'); }
function hideModal() { document.getElementById('pair-modal').classList.remove('open'); }
function togglePanel(name) {
    const c = document.getElementById(name+'-body');
    const a = document.getElementById(name+'-arrow');
    c.classList.toggle('open');
    a.innerText = c.classList.contains('open') ? '▲' : '▼';
}
function updVal(id) { document.getElementById(id+'-val').innerText = document.getElementById(id).value; }
function updMax(id) { document.getElementById(id+'-val').innerText = document.getElementById(id).value + '°'; }
function fmtUp(s) {
    if(s<60) return s+'s';
    if(s<3600) return Math.floor(s/60)+'m '+s%60+'s';
    return Math.floor(s/3600)+'h '+Math.floor(s%3600/60)+'m';
}
function setOffline() {
    document.getElementById('conn-status').innerText='离线';
    document.getElementById('conn-dot').classList.add('offline');
}
function setOnline() {
    document.getElementById('conn-status').innerText='在线';
    document.getElementById('conn-dot').classList.remove('offline');
}

// ===== 调试日志 =====
let debugLines = [];
function dbg(msg) {
    debugLines.push('['+new Date().toLocaleTimeString()+'] '+msg);
    if(debugLines.length>60) debugLines.shift();
    const el = document.getElementById('debug-log');
    if(el) { el.innerText = debugLines.join('\n'); el.scrollTop = el.scrollHeight; }
}

// ===== 映射配置 =====
const DEFAULT_MAP = {
    ttAxis: 0, ttInv: false,
    s1Axis: 1, s1Inv: false,
    s2Axis: 3, s2Inv: false,
    s3Open: '4,6', s3Close: '5,7', s3Center: '1',
    deadzone: 0.25
};
let gpMap = {};

function loadMapping() {
    try {
        const saved = localStorage.getItem('gpMapping');
        gpMap = saved ? JSON.parse(saved) : {...DEFAULT_MAP};
    } catch(e) { gpMap = {...DEFAULT_MAP}; }
    // 同步到 UI
    document.getElementById('map-tt-axis').value = gpMap.ttAxis;
    document.getElementById('map-tt-inv').checked = gpMap.ttInv;
    document.getElementById('map-s1-axis').value = gpMap.s1Axis;
    document.getElementById('map-s1-inv').checked = gpMap.s1Inv;
    document.getElementById('map-s2-axis').value = gpMap.s2Axis;
    document.getElementById('map-s2-inv').checked = gpMap.s2Inv;
    document.getElementById('map-s3-open').value = gpMap.s3Open;
    document.getElementById('map-s3-close').value = gpMap.s3Close;
    document.getElementById('map-s3-center').value = gpMap.s3Center;
    document.getElementById('map-deadzone').value = gpMap.deadzone;
}

function readMappingFromUI() {
    gpMap.ttAxis = +document.getElementById('map-tt-axis').value;
    gpMap.ttInv = document.getElementById('map-tt-inv').checked;
    gpMap.s1Axis = +document.getElementById('map-s1-axis').value;
    gpMap.s1Inv = document.getElementById('map-s1-inv').checked;
    gpMap.s2Axis = +document.getElementById('map-s2-axis').value;
    gpMap.s2Inv = document.getElementById('map-s2-inv').checked;
    gpMap.s3Open = document.getElementById('map-s3-open').value.trim();
    gpMap.s3Close = document.getElementById('map-s3-close').value.trim();
    gpMap.s3Center = document.getElementById('map-s3-center').value.trim();
    gpMap.deadzone = +document.getElementById('map-deadzone').value || 0.25;
}

function saveMapping() {
    readMappingFromUI();
    localStorage.setItem('gpMapping', JSON.stringify(gpMap));
    dbg('✅ 映射配置已保存');
    alert('映射已保存！');
}

function resetMapping() {
    gpMap = {...DEFAULT_MAP};
    localStorage.removeItem('gpMapping');
    loadMapping();
    dbg('↩ 映射已恢复默认');
    alert('已恢复默认映射！');
}

// 解析按钮编号字符串 "4,6" → [4, 6]
function parseBtns(str) {
    return str.split(',').map(s=>parseInt(s.trim())).filter(n=>!isNaN(n));
}

// 按钮自动绑定逻辑
let bindTarget = null;
function startBind(id) {
    if(bindTarget && bindTarget !== id) {
        document.getElementById(bindTarget).style.boxShadow = 'none';
    }
    bindTarget = id;
    const el = document.getElementById(id);
    el.style.boxShadow = '0 0 0 2px var(--success)';
}

// 页面加载时读取配置
loadMapping();

// ===== 核心API =====
let reqSeq = Date.now();

function sendApi(device, action, speed, duration) {
    speed = speed || 255; duration = duration || 0;
    
    let seq = reqSeq++;
    fetch('/api/control', {
        method:'POST', headers:{'Content-Type':'application/json'},
        body: JSON.stringify({device,action,speed,duration,seq})
    }).then(r=>{if(!r.ok)console.error('API error');}).catch(e=>{
        setOffline();
    });
}

// ===== 网页按钮控制 =====
function webDown(device, action) {
    let speed=255;
    if(device==='tt') speed = +document.getElementById('tt-speed').value;
    else if(device==='servo1') speed = +document.getElementById('s1-speed').value;
    else if(device==='servo2') speed = +document.getElementById('s2-speed').value;
    else if(device==='servo3') speed = +document.getElementById('s3-speed').value;
    sendApi(device, action, speed, 0);
}

function webUp(device) {
    sendApi(device, 'stop');
}

// ===== 状态轮询 =====
setInterval(()=>{
    fetch('/api/status').then(r=>r.json()).then(d=>{
        setOnline();
        document.getElementById('wifi-rssi').innerText = '📶 ' + d.rssi + 'dBm';
        document.getElementById('wifi-uptime').innerText = '⏱️ ' + fmtUp(d.uptime);
        if (d.servo3_angle !== undefined) {
            document.getElementById('servo3-angle-display').innerText = d.servo3_angle + '°';
        }
        if (d.tt_dir !== undefined) {
            document.getElementById('tt-state').innerText = '状态: ' + (d.tt_dir===1?'正转 (▲)':d.tt_dir===-1?'反转 (▼)':'停止');
            document.getElementById('tt-state').style.color = d.tt_dir===0?'':'var(--success)';
        }
        if (d.s1_dir !== undefined) {
            document.getElementById('s1-state').innerText = '状态: ' + (d.s1_dir===1?'右转 (▶)':d.s1_dir===-1?'左转 (◀)':'停止');
            document.getElementById('s1-state').style.color = d.s1_dir===0?'':'var(--success)';
        }
        if (d.s2_dir !== undefined) {
            document.getElementById('s2-state').innerText = '状态: ' + (d.s2_dir===1?'右转 (▶)':d.s2_dir===-1?'左转 (◀)':'停止');
            document.getElementById('s2-state').style.color = d.s2_dir===0?'':'var(--success)';
        }
        if (d.s3_dir !== undefined) {
            document.getElementById('s3-state').innerText = '状态: ' + (d.s3_dir===1?'闭合中 (▷)':d.s3_dir===-1?'张开中 (◁)':'静止');
            document.getElementById('s3-state').style.color = d.s3_dir===0?'':'var(--success)';
        }
        // 同步最大角度到滑块（仅当用户没有在拖动时同步，简化处理直接设置）
        const maxInput = document.getElementById('s3-max');
        if (d.servo3_max !== undefined && maxInput && document.activeElement !== maxInput) {
            maxInput.value = d.servo3_max;
            document.getElementById('s3-max-val').innerText = d.servo3_max + '°';
        }
    }).catch(()=>setOffline());
}, 200);

function sendMaxAngle() {
    const maxA = +document.getElementById('s3-max').value;
    fetch('/api/control', {
        method:'POST', headers:{'Content-Type':'application/json'},
        body: JSON.stringify({device:'servo3', action:'setmax', value:maxA})
    }).then(r=>{if(!r.ok)console.error('API error');}).catch(()=>setOffline());
}

// 阻止触摸默认
document.addEventListener('touchstart', e=>{
    if(e.target.classList.contains('btn-half')||e.target.classList.contains('btn-center'))
        e.preventDefault();
},{passive:false});

// ==========================================
// 🎮 手柄引擎 (使用可配置映射)
// ==========================================
let gpIdx = null;
// 手柄上一次状态
let gpLast = {
    tt: 'stop', ttSpd: 0, ttTime: 0,
    s1: 'stop', s1Spd: 0, s1Time: 0,
    s2: 'stop', s2Spd: 0, s2Time: 0,
    s3: 'stop'
};

window.addEventListener('gamepadconnected', e=>{
    gpIdx = e.gamepad.index;
    const name = e.gamepad.id.substring(0,20);
    document.getElementById('gamepad-status').innerText='🎮 '+name;
    document.getElementById('gp-badge').classList.add('connected');
    dbg('✅ 手柄已连接: '+e.gamepad.id);
    dbg('  轴: '+e.gamepad.axes.length+' / 按钮: '+e.gamepad.buttons.length);
    requestAnimationFrame(gpPoll);
});

window.addEventListener('gamepaddisconnected', e=>{
    if(e.gamepad.index===gpIdx) {
        gpIdx=null;
        document.getElementById('gamepad-status').innerText='🎮 未连接';
        document.getElementById('gp-badge').classList.remove('connected');
        dbg('❌ 手柄已断开');
    }
});

let pollFrame = 0;

function gpPoll() {
    if(gpIdx===null) return;
    const gp = navigator.getGamepads()[gpIdx];
    if(!gp) { requestAnimationFrame(gpPoll); return; }

    // 每次轮询都从 UI 读取最新映射（用户可能实时修改）
    readMappingFromUI();
    const DZ = gpMap.deadzone;

    // ===== 处理按键追加绑定模式 =====
    if(bindTarget) {
        for(let i=0; i<gp.buttons.length; i++) {
            if(gp.buttons[i].pressed) {
                const el = document.getElementById(bindTarget);
                let vals = el.value.split(',').map(s=>s.trim()).filter(s=>s!=='');
                if(!vals.includes(i.toString())) {
                    vals.push(i.toString());
                    el.value = vals.join(',');
                }
                el.style.boxShadow = 'none';
                bindTarget = null;
                // 自动保存
                saveMapping();
                break;
            }
        }
        // 如果处于绑定模式，暂停后续控制逻辑以防误触
        requestAnimationFrame(gpPoll);
        return;
    }

    // ===== 读取摇杆 =====
    function readAxis(idx, inv) {
        let v = (idx < gp.axes.length) ? gp.axes[idx] : 0;
        return inv ? -v : v;
    }

    let ttVal = readAxis(gpMap.ttAxis, gpMap.ttInv);
    let s1Val = readAxis(gpMap.s1Axis, gpMap.s1Inv);
    let s2Val = readAxis(gpMap.s2Axis, gpMap.s2Inv);

    // ===== 摇杆速度映射函数 (考虑死区，映射到 100-255) =====
    // 基础速度设为100是为了防止推杆刚过死区时电压过低导致电机堵转蜂鸣
    function mapSpeed(val) {
        let abs = Math.abs(val);
        if(abs <= DZ) return 0;
        return Math.min(255, Math.round(100 + (abs - DZ)/(1 - DZ) * 155));
    }

    let now = Date.now();

    // ===== TT电机 =====
    let ttState = 'stop', ttSpeed = 255;
    if(Math.abs(ttVal) > DZ) {
        ttState = ttVal < 0 ? 'down' : 'up';
        ttSpeed = mapSpeed(ttVal);
    }
    let ttStateChanged = ttState !== gpLast.tt;
    let ttSpeedChanged = ttState !== 'stop' && Math.abs(ttSpeed - gpLast.ttSpd) > 15;
    
    if (ttStateChanged || (ttSpeedChanged && now - gpLast.ttTime > 100)) {
        ttState==='stop' ? sendApi('tt','stop') : sendApi('tt',ttState,ttSpeed,0);
        gpLast.tt = ttState;
        gpLast.ttSpd = ttSpeed;
        gpLast.ttTime = now;
    }

    // ===== 舵机1 =====
    let s1State = 'stop', s1Speed = 255;
    if(Math.abs(s1Val) > DZ) {
        s1State = s1Val < 0 ? 'left' : 'right';
        s1Speed = mapSpeed(s1Val);
    }
    let s1StateChanged = s1State !== gpLast.s1;
    let s1SpeedChanged = s1State !== 'stop' && Math.abs(s1Speed - gpLast.s1Spd) > 15;

    if (s1StateChanged || (s1SpeedChanged && now - gpLast.s1Time > 100)) {
        s1State==='stop' ? sendApi('servo1','stop') : sendApi('servo1',s1State,s1Speed,0);
        gpLast.s1 = s1State;
        gpLast.s1Spd = s1Speed;
        gpLast.s1Time = now;
    }

    // ===== 舵机2 =====
    let s2State = 'stop', s2Speed = 255;
    if(Math.abs(s2Val) > DZ) {
        s2State = s2Val < 0 ? 'left' : 'right';
        s2Speed = mapSpeed(s2Val);
    }
    let s2StateChanged = s2State !== gpLast.s2;
    let s2SpeedChanged = s2State !== 'stop' && Math.abs(s2Speed - gpLast.s2Spd) > 15;

    if (s2StateChanged || (s2SpeedChanged && now - gpLast.s2Time > 100)) {
        s2State==='stop' ? sendApi('servo2','stop') : sendApi('servo2',s2State,s2Speed,0);
        gpLast.s2 = s2State;
        gpLast.s2Spd = s2Speed;
        gpLast.s2Time = now;
    }

    // ===== 舵机3 夹爪 (可配置按钮) =====
    let openBtns = parseBtns(gpMap.s3Open);
    let closeBtns = parseBtns(gpMap.s3Close);
    let centerBtns = parseBtns(gpMap.s3Center);

    let openPress = openBtns.some(i => i<gp.buttons.length && (gp.buttons[i].pressed || gp.buttons[i].value>0.5));
    let closePress = closeBtns.some(i => i<gp.buttons.length && (gp.buttons[i].pressed || gp.buttons[i].value>0.5));
    let centerPress = centerBtns.some(i => i<gp.buttons.length && gp.buttons[i].pressed);

    let s3State = 'stop';
    if(openPress) s3State='left';
    else if(closePress) s3State='right';
    else if(centerPress) s3State='center';

    if(s3State !== gpLast.s3) {
        if(s3State==='stop') sendApi('servo3','stop');
        else if(s3State==='center') sendApi('servo3','center');
        else sendApi('servo3', s3State, 180, 0);
        gpLast.s3 = s3State;
        if(s3State!=='stop') dbg('夹爪: '+(s3State==='left'?'张开':s3State==='right'?'闭合':'归中'));
    }

    // ===== 实时检测显示 =====
    pollFrame++;
    if(pollFrame % 6 === 0) {
        // 轴
        let axStr = '';
        for(let i=0; i<gp.axes.length; i++) {
            if(Math.abs(gp.axes[i])>0.05) axStr += 'A'+i+':'+gp.axes[i].toFixed(2)+' ';
        }
        document.getElementById('live-axes').innerText = axStr || '--';

        // 按钮
        let btnStr = '';
        for(let i=0; i<gp.buttons.length; i++) {
            if(gp.buttons[i].pressed || gp.buttons[i].value>0.3)
                btnStr += 'B'+i+'('+gp.buttons[i].value.toFixed(1)+') ';
        }
        document.getElementById('live-btns').innerText = btnStr || '--';
    }

    // 调试日志 (每60帧一次)
    if(pollFrame % 60 === 0) {
        let btnLog='';
        for(let i=0;i<gp.buttons.length;i++) {
            if(gp.buttons[i].pressed) btnLog+='B'+i+' ';
        }
        if(btnLog) dbg('按钮: '+btnLog);
    }

    requestAnimationFrame(gpPoll);
}
</script>
</body>
</html>
)rawliteral";
