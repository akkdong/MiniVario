package club.rascal.notorious.variolauncher;

import android.content.Context;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.net.Uri;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;

import java.util.List;

public class MainActivity extends AppCompatActivity {
    //
    private static String TAG = "MiniVario";
    private EditText editSSID = null;
    private EditText editPassword = null;
    private EditText editHostIP = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_connect);

        // Check wifi enabled or not
        WifiManager wifi = (WifiManager)this.getApplicationContext().getSystemService(Context.WIFI_SERVICE);

        if (! wifi.isWifiEnabled())
        {
            Toast.makeText(this, "WiFi is disabled enabling...", Toast.LENGTH_LONG).show();
            wifi.setWifiEnabled(true);
        }

        editSSID = (EditText)this.findViewById(R.id.edit_wifi_ssid);
        editPassword = (EditText)this.findViewById(R.id.edit_wifi_password);
        editHostIP = (EditText)this.findViewById(R.id.edit_host_ip);

        Button btnConnect = (Button)this.findViewById(R.id.btn_wifi_connect);
        btnConnect.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                //
                String strSSID = editSSID.getText().toString();
                String strPassword = editPassword.getText().toString();
                Log.v(TAG, "WiFi Connect: " + strSSID + ", " + strPassword);

                WifiConfiguration conf = new WifiConfiguration();

                conf.SSID = "\"" + strSSID + "\"";   // Please note the quotes. String should contain ssid in quotes
                conf.status = WifiConfiguration.Status.ENABLED;
                conf.priority = 40;
                conf.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
                conf.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
                conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
                conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
                conf.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
                conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
                conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
                conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
                conf.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
                conf.preSharedKey = "\"" + strPassword + "\"";

                WifiManager wifiManager = (WifiManager)view.getContext().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
                int networkId = wifiManager.addNetwork(conf);
                Log.v(TAG, "Add result " + networkId);

                List<WifiConfiguration> list = wifiManager.getConfiguredNetworks();
                for (WifiConfiguration i : list) {
                    if (i.SSID != null && i.SSID.equals("\"" + "MiniVario" + "\"")) {
                        Log.v(TAG, "WifiConfiguration SSID " + i.SSID);

                        boolean isDisconnected = wifiManager.disconnect();
                        Log.v(TAG, "isDisconnected : " + isDisconnected);

                        boolean isEnabled = wifiManager.enableNetwork(i.networkId, true);
                        Log.v(TAG, "isEnabled : " + isEnabled);

                        boolean isReconnected = wifiManager.reconnect();
                        Log.v(TAG, "isReconnected : " + isReconnected);

                        break;
                    }
                }

            }
        });

        Button btnPageOpen = (Button)this.findViewById(R.id.btn_page_open);
        btnPageOpen.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                String strHostIP = editHostIP.getText().toString();

                Log.v(TAG, "Open Configuration Page: http://" + strHostIP);
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://" + strHostIP));
                startActivity(intent);
            }
        });
    }
}
