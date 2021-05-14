#include <driver/gpio.h>
#include <driver/spi_slave.h>

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_NUM_33))
#define GPIO_HANDSHAKE GPIO_NUM_2
#define GPIO_MOSI GPIO_NUM_13
#define GPIO_MISO GPIO_NUM_12
#define GPIO_SCLK GPIO_NUM_14
#define GPIO_CS GPIO_NUM_15

#define RCV_HOST    HSPI_HOST
#define DMA_CHAN    2

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<GPIO_HANDSHAKE));
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1<<GPIO_HANDSHAKE));
}



WORD_ALIGNED_ATTR char sendbuf[129]="";
WORD_ALIGNED_ATTR char recvbuf[129]="";
spi_slave_transaction_t t;
int n=0;
esp_err_t ret;

void slave_setup()
{


    //Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=GPIO_MISO,
        .sclk_io_num=GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .spics_io_num=GPIO_CS,
        .flags=0,
        .queue_size=3,
        .mode=0,
        .post_setup_cb=my_post_setup_cb,
        .post_trans_cb=my_post_trans_cb
    };

    //Configuration for the handshake line
    gpio_config_t io_conf={
      .pin_bit_mask=(1<<GPIO_HANDSHAKE),
      .mode=GPIO_MODE_OUTPUT,
      .pull_up_en=GPIO_PULLUP_DISABLE,
      .pull_down_en=GPIO_PULLDOWN_DISABLE,
      .intr_type=GPIO_INTR_DISABLE
    };

    //Configure handshake line as output
    gpio_config(&io_conf);
    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    ret=spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, DMA_CHAN);
    assert(ret==ESP_OK);

    memset(recvbuf, 0, 33);
    memset(&t, 0, sizeof(t));
}


void test_idf_setup()
{
  gpio_config_t io_conf;
  //disable interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  //set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  //bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  //disable pull-down mode
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  //disable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  //configure GPIO with the given settings
  gpio_config(&io_conf);
}
#define SPI_ONE_PACKET_SIZE 2400
//uint32_t sizz=2400;
uint8_t tbuf[4800];

bool packettranmit(uint8_t *tbuf) 
{
  uint8_t rbuf[2400];
  t.length=SPI_ONE_PACKET_SIZE*8;
  t.tx_buffer=tbuf;
  t.rx_buffer=rbuf;

  ret=spi_slave_transmit(RCV_HOST, &t, 20);
  if(ret)
  {
    printf("No ask1!\r\n");
    return false;
  }

  t.length=SPI_ONE_PACKET_SIZE*8;
  t.tx_buffer=tbuf+SPI_ONE_PACKET_SIZE;
  ret=spi_slave_transmit(RCV_HOST, &t, 30);
  if(ret)
  {
    printf("No ask2!\r\n");
    return false;
  }

  return true;
}

void spi_receive()
{
  
  //Clear receive buffer, set send buffer to something sane
    //memset(recvbuf, 0x00, 129);
    //memset(sendbuf,0,129);

    
    //uint8_t rbuf[sizz];
    
    uint32_t ptime=millis();
    memset(tbuf,43,4800);
    packettranmit(tbuf);
    uint32_t mctime =millis();
    //printf("time :%d \r\n",(mctime-ptime));

    
    
    // for(int i=0;i<2;i++)
    // {
    //   memset(tbuf,45,sizz*2);
    //   memset(rbuf,0,sizz);
    //   t.length=sizz*8;
    //   t.tx_buffer=tbuf+i*sizz;
    //   t.rx_buffer=rbuf;
        
    //     ret=spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
    //       if(ret)
    //       {
    //         printf("No ask!\r\n");
    //         return;
    //       }else{
            
    //       }
    //       printf("Received: %d\n", rbuf[0]);
    // }
    

    


 //   sprintf(sendbuf, "This is the receiver, sending data for transmission number %04d.", n);

    // union{
    //   uint8_t lenpack[4];
    //   uint32_t lenpakss;
    // }tm;
    // tm.lenpakss = 100;

    // t.length=4*8;
    // t.tx_buffer=tm.lenpack;
    // t.rx_buffer=recvbuf;

    // ret=spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
    // if(ret)
    // {
    //   printf("No ask!\r\n");
    //   return;
    // }

    
    // //Set up a transaction of 128 bytes to send/receive
    // t.length=100*8;
    // t.tx_buffer=sendbuf;
    // t.rx_buffer=recvbuf;
    // /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
    // initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
    // by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
    // .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
    // data.
    // */
    // ret=spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);

    // //spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
    // //received data from the master. Print it.
    // printf("Received: %d\n", ret);
    // n++;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);                     // Start serial communication 
  Serial.println("start");
  slave_setup();

}

void loop() {
  // put your main code here, to run repeatedly:
  spi_receive();
}
