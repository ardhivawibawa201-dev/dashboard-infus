
import { useState, useEffect, useRef } from "react";
import mqtt from "mqtt";
import "./App.css";

function App() {
  const [status, setStatus] = useState("Offline");
  const [berat, setBerat] = useState(0);
  const [tpm, setTpm] = useState(0);
  const [pwm, setPwm] = useState(0);
  const [modeAktif, setModeAktif] = useState("-");
  const [statusInfus, setStatusInfus] = useState("NORMAL");

  const [isLogin, setIsLogin] = useState(false);
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");

  const [modeTerpilih, setModeTerpilih] = useState("");
  const [pesanKontrol, setPesanKontrol] = useState("");

  const clientRef = useRef(null);

  // ==============================
  // KONEKSI MQTT
  // ==============================
  useEffect(() => {
    const client = mqtt.connect(
      "wss://870c766cdeac4276a1d1addf8b853c55.s1.eu.hivemq.cloud:8888/mqtt",
      {
        username: "Infuspintar",
        password: "Infuspintar1234_",
        reconnectPeriod: 3000,
        connectTimeout: 10000,
      }
    );

    clientRef.current = client;

    client.on("connect", () => {
      setStatus("Online");
      console.log("MQTT Connected");

      client.subscribe([
        "infus/berat",
        "infus/tpm",
        "infus/status",
        "infus/pwm",
        "infus/mode_status",
      ]);
    });

    client.on("offline", () => {
      setStatus("Offline");
    });

    client.on("close", () => {
      setStatus("Offline");
    });

    client.on("error", (error) => {
      console.error("MQTT Error:", error);
      setStatus("Offline");
    });

    client.on("message", (topic, message) => {
      const data = message.toString();

      if (topic === "infus/berat") {
        setBerat(data);
      }

      if (topic === "infus/tpm") {
        setTpm(data);
      }

      if (topic === "infus/status") {
        setStatusInfus(data);
      }

      if (topic === "infus/pwm") {
        setPwm(data);
      }

      if (topic === "infus/mode_status") {
        setModeAktif(data);

        if (["A", "B", "C"].includes(data)) {
          setModeTerpilih(data);
          setPesanKontrol(
            "ESP32 mengonfirmasi Mode " + data + " aktif."
          );
        } else if (data === "OFF") {
          setModeTerpilih("");
          setPesanKontrol("Motor dalam keadaan OFF.");
        }
      }
    });

    return () => {
      client.end();
      clientRef.current = null;
    };
  }, []);

  // ==============================
  // LOGIN
  // ==============================
  function login() {
    if (username === "infus123" && password === "infus123") {
      setIsLogin(true);
    } else {
      alert("Username atau Password Salah");
    }
  }

  // ==============================
  // KONTROL MODE MOTOR
  // ==============================
  function kirimMode(mode) {
    if (!clientRef.current || !clientRef.current.connected) {
      setPesanKontrol("MQTT belum terhubung. Periksa koneksi.");
      return;
    }

    setPesanKontrol(
      "Mengirim perintah Mode " + mode + " ke ESP32..."
    );

    clientRef.current.publish(
      "infus/mode",
      mode,
      { qos: 1 },
      (error) => {
        if (error) {
          setPesanKontrol("Gagal mengirim perintah.");
          return;
        }

        setPesanKontrol(
          "Perintah terkirim. Menunggu konfirmasi dari ESP32..."
        );
      }
    );
  }

  // ==============================
  // LOGIN PAGE
  // ==============================
  if (!isLogin) {
    return (
      <div className="login-container">
        <div className="login-card">
          <div className="login-logo">💧</div>

          <h1 className="login-title">SMART INFUS</h1>

          <p className="login-subtitle">
            Sistem Monitoring Infus Berbasis Internet of Things
          </p>

          <form
            onSubmit={(e) => {
              e.preventDefault();
              login();
            }}
          >
            <input
              className="login-input"
              type="text"
              placeholder="Masukkan username"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              autoComplete="username"
              required
            />

            <input
              className="login-input"
              type="password"
              placeholder="Masukkan password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              autoComplete="current-password"
              required
            />

            <button className="login-button" type="submit">
              LOGIN
            </button>
          </form>

          <p className="login-subtitle" style={{ marginBottom: 0, marginTop: 22 }}>
            Smart Infus Monitoring Dashboard
          </p>
        </div>
      </div>
    );
  }

  // ==============================
  // STATUS INFUS
  // ==============================
  let statusClass = "status-indicator";
  let statusLabel = statusInfus;

  if (statusInfus === "NORMAL") {
    statusLabel = "NORMAL";
  } else if (statusInfus === "HAMPIR HABIS") {
    statusClass += " warning";
    statusLabel = "HAMPIR HABIS";
  } else if (
    statusInfus === "INFUS MACET" ||
    statusInfus === "MACET"
  ) {
    statusClass += " danger";
    statusLabel = "INFUS MACET";
  }

  // ==============================
  // DASHBOARD
  // ==============================
  return (
    <div className="dashboard">
      <header className="dashboard-header">
        <div className="dashboard-brand">
          <h1>SMART INFUS</h1>
          <p>Monitoring dan Kontrol Infus Berbasis IoT</p>
        </div>

        <div className="connection-badge">
          <span
            className={
              status === "Online"
                ? "connection-dot online"
                : "connection-dot"
            }
          />
          MQTT {status}
        </div>
      </header>

      <main className="dashboard-content">
        <h2 className="section-heading">
          Dashboard Monitoring
        </h2>

        <p className="section-description">
          Ringkasan pembacaan sensor dan kondisi sistem.
        </p>

        <div className="monitoring-grid">
          <div className="monitor-card">
            <div className="monitor-card-header">
              <div className="monitor-icon">⚖️</div>
              <div>
                <p className="monitor-label">Berat Cairan</p>
                <p className="monitor-label">Pembacaan load cell</p>
              </div>
            </div>

            <p className="monitor-value">
              {berat}
              <span className="monitor-unit">gram</span>
            </p>
          </div>

          <div className="monitor-card">
            <div className="monitor-card-header">
              <div className="monitor-icon">💧</div>
              <div>
                <p className="monitor-label">Laju Tetesan</p>
                <p className="monitor-label">Pembacaan sensor tetes</p>
              </div>
            </div>

            <p className="monitor-value">
              {tpm}
              <span className="monitor-unit">TPM</span>
            </p>
          </div>

          <div className="monitor-card">
            <div className="monitor-card-header">
              <div className="monitor-icon">⚙️</div>
              <div>
                <p className="monitor-label">PWM Motor</p>
                <p className="monitor-label">Nilai kendali motor</p>
              </div>
            </div>

            <p className="monitor-value">
              {pwm}
              <span className="monitor-unit">/ 255</span>
            </p>
          </div>

          <div className="monitor-card">
            <div className="monitor-card-header">
              <div className="monitor-icon">🎛️</div>
              <div>
                <p className="monitor-label">Mode Motor</p>
                <p className="monitor-label">Konfirmasi dari ESP32</p>
              </div>
            </div>

            <p className="monitor-value">
              {modeAktif}
            </p>
          </div>
        </div>

        <section className="status-panel">
          <h2>📋 Status Sistem Infus</h2>

          <span className={statusClass}>
            {statusLabel}
          </span>

          <p className="control-description" style={{ marginTop: 14, marginBottom: 0 }}>
            Status ini mengikuti data yang dikirimkan ESP32.
          </p>
        </section>

        <section className="control-panel">
          <h2>🎛️ Kontrol Motor Infus</h2>

          <p className="control-description">
            Pilih mode untuk mengirim perintah pengoperasian motor ke ESP32.
          </p>

          <div className="mode-grid">
            <button
              className={
                modeTerpilih === "A"
                  ? "mode-button active"
                  : "mode-button"
              }
              onClick={() => kirimMode("A")}
              disabled={status !== "Online"}
            >
              <span className="mode-name">Mode A</span>
              <span className="mode-detail">
                Tanpa motor
                <br />
                PWM 0
              </span>
            </button>

            <button
              className={
                modeTerpilih === "B"
                  ? "mode-button active"
                  : "mode-button"
              }
              onClick={() => kirimMode("B")}
              disabled={status !== "Online"}
            >
              <span className="mode-name">Mode B</span>
              <span className="mode-detail">
                Kecepatan motor
                <br />
                PWM 120
              </span>
            </button>

            <button
              className={
                modeTerpilih === "C"
                  ? "mode-button active"
                  : "mode-button"
              }
              onClick={() => kirimMode("C")}
              disabled={status !== "Online"}
            >
              <span className="mode-name">Mode C</span>
              <span className="mode-detail">
                Kecepatan motor
                <br />
                PWM 130
              </span>
            </button>
          </div>

          {pesanKontrol && (
            <div className="control-message" role="status">
              {pesanKontrol}
            </div>
          )}
        </section>

        <p
          style={{
            textAlign: "center",
            color: "#64748b",
            fontSize: 12,
            marginTop: 25,
          }}
        >
          Smart Infus Monitoring · Prototipe pemantauan dan pengujian
        </p>
      </main>
    </div>
  );
}

export default App;
