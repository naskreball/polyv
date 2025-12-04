package com.example.polivsystem;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    // Загрузка библиотеки
    static {
        System.loadLibrary("polivsystem");
    }

    // Нативные методы
    public native String connectESP();
    public native void disconnectESP();
    public native String getStatus();
    public native String getSoil();
    public native String pumpOn();
    public native String pumpOff();
    public native String autoOn();
    public native String autoOff();
    public native String shortPump();
    public native boolean isConnected();

    // UI элементы
    private TextView statusText;
    private Button btnConnect;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Находим все элементы
        statusText = findViewById(R.id.statusText);
        btnConnect = findViewById(R.id.btnConnect);
        Button btnStatus = findViewById(R.id.btnStatus);
        Button btnSoil = findViewById(R.id.btnSoil);
        Button btnShortPump = findViewById(R.id.btnShortPump);
        Button btnPumpOn = findViewById(R.id.btnPumpOn);
        Button btnPumpOff = findViewById(R.id.btnPumpOff);
        Button btnAutoOn = findViewById(R.id.btnAutoOn);
        Button btnAutoOff = findViewById(R.id.btnAutoOff);

        // Настройка кнопок
        setupButtons(btnStatus, btnSoil, btnShortPump, btnPumpOn,
                btnPumpOff, btnAutoOn, btnAutoOff);

        // Обновление интерфейса
        updateUI();
    }

    private void setupButtons(Button btnStatus, Button btnSoil, Button btnShortPump,
                              Button btnPumpOn, Button btnPumpOff,
                              Button btnAutoOn, Button btnAutoOff) {

        // Подключение/отключение
        btnConnect.setOnClickListener(v -> {
            new Thread(() -> {
                if (isConnected()) {
                    disconnectESP();
                    runOnUiThread(() -> {
                        showToast("Отключено");
                        updateUI();
                    });
                } else {
                    String result = connectESP();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateUI();
                    });
                }
            }).start();
        });

        // Статус
        btnStatus.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String status = getStatus();
                    runOnUiThread(() -> statusText.setText("Статус: " + status));
                }).start();
            }
        });

        // Влажность
        btnSoil.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String soil = getSoil();
                    runOnUiThread(() -> statusText.setText("Влажность: " + soil));
                }).start();
            }
        });

        // Короткий полив (1 секунда)
        btnShortPump.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String result = shortPump();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateStatus();
                    });
                }).start();
            }
        });

        // Длинный полив (3 секунды)
        btnPumpOn.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String result = pumpOn();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateStatus();
                    });
                }).

                start();
            }
        });

        // Стоп
        btnPumpOff.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String result = pumpOff();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateStatus();
                    });
                }).start();
            }
        });

        // Авто ВКЛ
        btnAutoOn.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String result = autoOn();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateStatus();
                    });
                }).start();
            }
        });

        // Авто ВЫКЛ
        btnAutoOff.setOnClickListener(v -> {
            if (checkConnection()) {
                new Thread(() -> {
                    String result = autoOff();
                    runOnUiThread(() -> {
                        showToast(result);
                        updateStatus();
                    });
                }).start();
            }
        });
    }

    private boolean checkConnection() {
        if (!isConnected()) {
            showToast("Сначала подключитесь!");
            return false;
        }
        return true;
    }

    private void updateStatus() {
        if (!isConnected()) return;

        new Thread(() -> {
            String status = getStatus();
            runOnUiThread(() -> statusText.setText("Статус: " + status));
        }).start();
    }

    private void updateUI() {
        runOnUiThread(() -> {
            boolean connected = isConnected();

            btnConnect.setText(connected ? "Отключиться" : "Подключиться");

            // Все кнопки управления
            int[] controlButtons = {
                    R.id.btnStatus, R.id.btnSoil,
                    R.id.btnShortPump, R.id.btnPumpOn,
                    R.id.btnPumpOff, R.id.btnAutoOn,
                    R.id.btnAutoOff
            };

            for (int btnId : controlButtons) {
                Button btn = findViewById(btnId);
                if (btn != null) {
                    btn.setEnabled(connected);
                }
            }

            if (!connected) {
                statusText.setText("Не подключено");
            }
        });
    }

    private void showToast(String message) {
        runOnUiThread(() ->
                Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
        );
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnectESP();
    }
}