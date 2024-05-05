# ESP32’nin TCP Client Bağlantısı

Bu yazıda IoT uygulamalarında sıklıkla Wi-Fi modül olarak kullanılan ESP32’nin **AT komutları** kullanılarak client modda TCP bağlantısı yapılabilmesi gereken işlemeleri uygulamaları ile anlatacağım. İyi okumalar!

---

### ESP32 ve MCU Arasında Gerekli Olan Bağlantılar

Bu işlem gerçekleştirilmeden önce ESP32’nin içerisinde yazılım var mı diye kontol edilir. Bu sağlamlık kontrolünü Arduino IDE’si veya bir USB-TTL aracılığıyla gerçekleştirebilirisiniz. ESP32 ve MCU arasında sağlanması gereken bağlantılar:

ESP32 Power → MCU GPIO_OUT

ESP32 GND → MCU GND

ESP32 UART RX → MCU UART TX

ESP32 UART TX → MCU UART RX

Bu bağlantılar sağlandıktan sonra artık MCU’muzun gerekli olan ayarlamalarını yapabiliriz.

---

### AT Komutları

ESP32’nin WiFi ve Bluetooth işlevlerini kontrol etmek için kullanılan bir dizi komuttur. Bu komutlar, ESP32’nizin ayarlarını değiştirmek, bilgileri sorgulamak ve çeşitli işlemleri gerçekleştirmek için kullanılır.

ESP32’nin TCP bağlantısının sağlanabilmesi için sırasıyla aşağıdaki komutların gönderilmesi gerekmektedir.

- WiFi özelliğinin aktif edilmesi için;

```markdown
AT+CWMODE=3
```

Komutu gönderilmedlidir. Bu komut başarılı bir biçim de ESP32’ye gönderilmesi durumun da ESP32 aşağıdaki mesajları bize sırasıyla iletecektir.

```c
+CWMODE:3
OK
```

- WiFi bağlantısnın sağlanabilmesi için:

```c
AT+CWJAP="wifi_ssid","wifi_password"
```

Yukarıdaki komutun içindeki parametreler bağlanması istenilen WiFi ismini “wifi_ssid” kısmına ve şifreyi ise “wifi_password” kısmına girilmelidir. Komut başarılı bir biçim de ESP32’ye gönderilmesi durumun da ESP32 aşağıdaki mesajları bize sırasıyla iletecektir.

```c
WIFI CONNECTED
WIFI GOT IP
```

- TCP client bağlantısının yapılabilmesi için:

```c
AT+CIPSTART="TCP","tcp_ip",tcp_port
```

Yukarıdaki komutun içindeki parametreler bağlanması istenilen serverın IP bilgisi “tcp_ip” kısmına ve port bilgisini ise “tcp” kısmına girilmelidir. Komut başarılı bir biçim de ESP32’ye gönderilmesi durumun da ESP32 aşağıdaki mesajları bize sırasıyla iletecektir.

```c
CONNECT
OK
```

---

### API’nin Çalışması İçin Yapılması Gerekenler

ESP’nin TCP bağlantısı yapılabilmesi için hazırlanmış olan uygulama dosyasının kaynak dosyasına baktığınızda tanımlanmış olan “pointer function” göreceksiniz. Tanımlanmış olan bu fonksiyonlar sırasıyla takip edilmelidir!

- ESP32’nin bağlı olduğu pini aktif etmek için;

```c
static void power_enable_fp(void * esp_api_object)
```

Bu fonksyion içerisine işlemcinizin bir pini aktif eden fonksiyonu yazmalısınız. Bu fonksiyon sayesinde ESP’nin API kısmında bu pin aktif edilecektir 

- ESP32’nin bağlı olduğu pini pasif hale getirmek için;

```c
static void power_disable_fp(void * esp_api_object)
```

Bu fonksyion içerisine işlemcinizin bir pini pasif hale getiren fonksiyonu yazmalısınız. Bu fonksiyon sayesinde ESP’nin API kısmında bu pasif moda getirmek için kullanılacaktır.

- ESP32’den gelen mesajların işlenmesi için;

```c
queue_write_data(&_app._api.rx_queue, (unsigned char *)rx_data, rx_data_length);
```

Yukarıdaki fonksiyon ile ESP32 tarafından gönderilen mesajalar işlenmesi için ESP API’nin kuyruk tampınuna yazılmış olacaktır.

- ESP32’ye mesaj gönderilmesi için;

```c
static void transmit_data_fp(void * called_by_object, void * esp_api_object, const unsigned char * rx_data, unsigned int rx_data_length)
```

Bu fonksiyon içerisine işlemcinizin bağlı olduğu UART hattına mesaj atan fonksiyonu çağırılması gerekmektedir.

---

### Bağlantı Ayarlarının Yapılması

Aşağıdaki fonksiyonlar ihtiyaca uygun şekilde doldurulmalıdır. 

- “WiFi” bilgileri;

```c
esp_return_type_e esp_set_wifi_ssid(esp_api_t * self, char * wifi_ssid);
esp_return_type_e esp_set_wifi_password(esp_api_t * self, char * wifi_password);
```

Verilmiş olan bu fonksiyonlara sırasıyla bağalancak “WiFi” ağının ismi ve şifresi girilmelidir.

- “TCP Server” bilgileri;

```c
esp_return_type_e esp_set_tcp_ip(esp_api_t * self, char * tcp_ip);
esp_return_type_e esp_set_tcp_port(esp_api_t * self, int  tcp_port);
```

Bu fonksiyonlara ise sırasıyla “TCP Server” IP ve port bilgileri girilmelidir.

Verilmiş olan bu ayarlar dışında kullandığınız internet ağında bir ip kısıtlaması veya firewall gibi önlemleriniz varsa aşağıdaki fonksiyonlara gerekli parametreleri tanımlayarak kullanabilirsiniz.

```c
esp_return_type_e esp_set_mac_ip(esp_api_t * self, char * mac_ip);
esp_return_type_e esp_set_static_ip(esp_api_t * self, char * static_ip);
esp_return_type_e esp_set_gateway_ip(esp_api_t * self, char * gateway_ip);
```

---

### Timer Fonksiyonun Tanımlanması

Aşağıdaki fonksiyon 1 milisaniyede bir çağırılan SysTick fonksiyonu veya observer pattern ile oluşturulmuş bir zamanlayıcı içerisinde çağırılmalıdır.

```c
void esp_api_1_ms_timer(esp_api_t * self);
```

---

### Create Fonksiyonun Tanımlanması

ESP API için oluşturulmuş olan create fonksiyonuna 3 adet paremetre girilmelidir. Bu parametreler;

- Çağırıldığı objenin adresi
- ESP API objesinin adresi
- Callback fonksiyonlarının adresi

```c
esp_return_type_e esp_api_create(void 		   * called_by_object,
																		 esp_api_t  * self,
																		 esp_api_cb * callback);
```

Fonksiyonun içerisine bu parametrelerin gönderilmesiyle birlikte ESP32 artık bir sunucuya bağlanmak için hazır halde olacaktır.

---

### Sunucu Bağlantısının Gerçekleştirilmesi

Sırasıyla AT komutlarını göndermemiz ve bu komutlara karşılık, ESP32’nin  vereceği cevapları beklememiz gerekmetedir. Bunun için aşağıdaki fonksyion kullanılmıştır.

Bu fonksiyona gerekli parametre değerleri girildikten sonra bağlantının sağlanması için gerekli işlemleri yapacaktır.

```c
void esp_api_tcp_api_controller(esp_api_t * self)
```

---
