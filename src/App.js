
import { useState, useEffect, useRef } from "react";
import mqtt from "mqtt";
import "./App.css";

const MQTT_URL =
  "wss://870c766cdeac4276a1d1addf8b853c55.s1.eu.hivemq.cloud:8884/mqtt";

// Isi dengan kredensial MQTT HiveMQ yang valid.
const MQTT_USERNAME = "Infuspintar";
const MQTT_PASSWORD = "GANTI_DENGAN_PASSWORD_MQTT_KAMU";

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

  // =========================
  // KONEKSI MQTT
  // =========================
  useEffect(() => {
    const client = mqtt.connect(MQTT_URL, {
      username: MQTT_USERNAME,
      password: MQTT_PASSWORD,
      reconnectPeriod: 3000,
      connectTimeout: 10000,
      clean: true,
    });

    clientRef.current = client;

    client.on("connect", () => {
      console.log("MQTT Connected");
      setStatus("Online");

      client.subscribe(
        [
          "infus/berat",
          "infus/tpm",
          "infus/status",
          "infus/pwm",
          "infus/mode_status",
        ],
        (error) => {
          if (error) {
            console.error("Gagal subscribe:", error);
          }
        }
      );
    });

    client.on("reconnect", () => {
      console.log("Mencoba menyambungkan ulang MQTT...");
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
            `ESP32 mengonfirmasi Mode ${data} aktif.`
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

  // =========================
  // LOGIN
  // =========================
  function login(event) {
    event.preventDefault();

    if (
      username === "infus123" &&
      password === "infus123"
    ) {
      setIsLogin(true);
    } else {
      alert("Username atau Password salah.");
    }
  }

  // =========================
  // KONTROL MOTOR
  // =========================
  function kirimMode(mode) {
    const client = clientRef.current;

    if (!client || !client.connected) {
      setPesanKontrol(
        "MQTT belum terhubung. Periksa koneksi terlebih dahulu."
      );
      return;
    }

    setPesanKontrol(
      `Mengirim perintah Mode ${mode} ke ESP32...`
    );

    client.publish(
      "infus/mode",
      mode,
      { qos: 1 },
      (error) => {
        if (error) {
          setPesanKontrol(
            "Gagal mengirim perintah ke MQTT."
          );
          return;
        }

        setPesanKontrol(
          `Perintah Mode ${mode} terkirim. Menunggu konfirmasi ESP32...`
        );
      }
    );
  }

  // =========================
  // TAMPILAN LOGIN
  // =========================
  if (!isLogin) {
    return (
      <div className="login-container">
        <div className="login-card">
          <div className="login-logo">💧</div>

          <h1 className="login-title">SMART INFUS</h1>

          <p className="login-subtitle">
            Sistem Monitoring Infus Berbasis IoT
          </p>

          <form onSubmit={login}>
            <input
              className="login-input"
              type="text"
              placeholder="Username"
              value={username}
              onChange={(event) =>
                setUsername(event.target.value)
              }
              autoComplete="username"
              required
            />

            <input
              className="login-input"
              type="password"
              placeholder="Password"
              value={password}
              onChange={(event) =>
                setPassword(event.target.value)
              }
              autoComplete="current-password"
              required
            />

            <button
              className="login-button"
              type="submit"
            >
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

  // =========================
  // STATUS INFUS
  // =========================
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

  // =========================
  // DASHBOARD
  // =========================
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
                <p className="monitor-label">
                  Pembacaan load cell
                </p>
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
                <p className="monitor-label">
                  Pembacaan sensor tetes
                </p>
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
                <p className="monitor-label">
                  Nilai kendali motor
                </p>
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
                <p className="monitor-label">
                  Konfirmasi dari ESP32
                </p>
              </div>
            </div>

            <p className="monitor-value">{modeAktif}</p>
          </div>
        </div>

        <section className="status-panel">
          <h2>📋 Status Sistem Infus</h2>

          <span className={statusClass}>
            {statusLabel}
          </span>

          <p
            className="control-description"
            style={{ marginTop: 14, marginBottom: 0 }}
          >
            Status mengikuti data yang dikirimkan ESP32.
          </p>
        </section>

        <section className="control-panel">
          <h2>🎛️ Kontrol Motor Infus</h2>

          <p className="control-description">
            Pilih mode untuk mengirim perintah pengoperasian
            motor ke ESP32.
          </p>

          <div className="mode-grid">
            <button
              type="button"
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
              type="button"
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
              type="button"
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
          Smart Infus Monitoring · Prototipe pengujian
        </p>
      </main>
    </div>
  );
}

export default App;
