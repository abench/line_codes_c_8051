 void test_dac()
{
  unsigned char i = 0;

  lefti = 0x0d; righti = 0xac; new_dotsi = _00000001;

start:
  do {
    do {
      pDAC = i++;
    } while(i);

    do {
      pDAC = --i;
    } while(i);
  } while (key_scan() == 12);
}
