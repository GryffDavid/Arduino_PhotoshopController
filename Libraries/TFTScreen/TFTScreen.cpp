#define SUPPORT_8352B
#define OFFSET_9327 32             //costs about 103 bytes, 0.08s

#include "TFTScreen.h"
#include "TFTScreen_shield.h"

#define wait_ms(ms)  delay(ms)
#define MIPI_DCS_REV1   (1<<0)
#define AUTO_READINC    (1<<1)
#define READ_BGR        (1<<2)
#define READ_LOWHIGH    (1<<3)
#define READ_24BITS     (1<<4)
#define XSA_XEA_16BIT   (1<<5)
#define READ_NODUMMY    (1<<6)
#define INVERT_GS       (1<<8)
#define INVERT_SS       (1<<9)
#define MV_AXIS         (1<<10)
#define INVERT_RGB      (1<<11)
#define REV_SCREEN      (1<<12)
#define FLIP_VERT       (1<<13)
#define FLIP_HORIZ      (1<<14)
#define TFTLCD_DELAY8 0xFF

TFTScreen::TFTScreen(int CS, int RS, int WR, int RD, int RST):Adafruit_GFX(240, 400)
{
    // we can not access GPIO pins until AHB has been enabled.
}

static uint8_t done_reset, is8347;

void TFTScreen::reset(void)
{
	done_reset = 1;
	setWriteDir();
	CTL_INIT();
	CS_IDLE;
	RD_IDLE;
	WR_IDLE;

	digitalWrite(5, LOW);
	delay(200);
	digitalWrite(5, HIGH);

	WriteCmdData(0xB0, 0x0000);
}

void TFTScreen::WriteCmdData(uint16_t cmd, uint16_t dat)
{
	CS_ACTIVE;
	WriteCmd(cmd);
	WriteData(dat);
	CS_IDLE;
}

static void WriteCmdParamN(uint16_t cmd, int8_t N, uint8_t * block)
{
	CS_ACTIVE;
	WriteCmd(cmd);

	while (N-- > 0)
	{
		uint8_t u8 = *block++;
		CD_DATA;
		write8(u8);
	}

	CS_IDLE;
}

static inline void WriteCmdParam4(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4)
{
	uint8_t d[4];
	d[0] = d1, d[1] = d2, d[2] = d3, d[3] = d4;
	WriteCmdParamN(cmd, 4, d);
}

void TFTScreen::pushCommand(uint16_t cmd, uint8_t * block, int8_t N) { WriteCmdParamN(cmd, N, block); }

static uint16_t read16bits(void)
{
	uint16_t ret;
	uint8_t lo;
	READ_8(ret);
	READ_8(lo);
	return (ret << 8) | lo;
}

uint32_t readReg40(uint16_t reg)
{
	uint16_t h, m, l;
	CS_ACTIVE;
	WriteCmd(reg);
	setReadDir();
	CD_DATA;
	h = read16bits();
	m = read16bits();
	l = read16bits();
	RD_IDLE;
	CS_IDLE;
	setWriteDir();
	return ((uint32_t)h << 24) | (m << 8) | (l >> 8);
}

uint16_t TFTScreen::readReg(uint16_t reg)
{
	uint16_t ret;
	uint8_t lo;

	if (!done_reset)
		reset();

	CS_ACTIVE;
	WriteCmd(reg);
	setReadDir();
	CD_DATA;
	ret = read16bits();
	RD_IDLE;
	CS_IDLE;
	setWriteDir();
	return ret;
}

uint32_t TFTScreen::readReg32(uint16_t reg)
{
	uint16_t h, l;
	CS_ACTIVE;
	WriteCmd(reg);
	setReadDir();
	CD_DATA;
	h = read16bits();
	l = read16bits();
	RD_IDLE;
	CS_IDLE;
	setWriteDir();
	return ((uint32_t)h << 16) | (l);
}

uint16_t TFTScreen::readID(void)
{
	return 0x65;
}

int16_t TFTScreen::readGRAM(int16_t x, int16_t y, uint16_t * block, int16_t w, int16_t h)
{
	uint16_t ret, dummy, _MR = _MW;
	int16_t n = w * h, row = 0, col = 0;
	uint8_t r, g, b, tmp;

	setAddrWindow(x, y, x + w - 1, y + h - 1);

	while (n > 0)
	{
		if (!(_lcd_capable & MIPI_DCS_REV1))
		{
			WriteCmdData(_MC, x + col);
			WriteCmdData(_MP, y + row);
		}

		CS_ACTIVE;
		WriteCmd(_MR);
		setReadDir();
		CD_DATA;

		if (_lcd_capable & READ_NODUMMY)
		{
			;
		}

		while (n)
		{
			if (_lcd_capable & READ_24BITS)
			{
				READ_8(r);
				READ_8(g);
				READ_8(b);

				ret = color565(r, g, b);
			}

			*block++ = ret;
			n--;

			if (!(_lcd_capable & AUTO_READINC))
				break;
		}

		if (++col >= w)
		{
			col = 0;
			if (++row >= h)
				row = 0;
		}

		RD_IDLE;
		CS_IDLE;
		setWriteDir();
	}

	if (!(_lcd_capable & MIPI_DCS_REV1))
		setAddrWindow(0, 0, width() - 1, height() - 1);

	return 0;
}

void TFTScreen::setRotation(uint8_t r)
{
	uint16_t GS, SS, ORG, REV = _lcd_rev;
	uint8_t val, d[3];
	rotation = r & 3;           // just perform the operation ourselves on the protected variables
	_width = (rotation & 1) ? HEIGHT : WIDTH;
	_height = (rotation & 1) ? WIDTH : HEIGHT;

	switch (rotation)
	{
	case 0:                    //PORTRAIT:
		val = 0x48;             //MY=0, MX=1, MV=0, ML=0, BGR=1
		break;
	case 1:                    //LANDSCAPE: 90 degrees
		val = 0x28;             //MY=0, MX=0, MV=1, ML=0, BGR=1
		break;
	case 2:                    //PORTRAIT_REV: 180 degrees
		val = 0x88;             //MY=1, MX=0, MV=0, ML=1, BGR=1
		break;
	case 3:                    //LANDSCAPE_REV: 270 degrees
		val = 0xF8;             //MY=1, MX=1, MV=1, ML=1, BGR=1
		break;
	}

	if (_lcd_capable & INVERT_GS)
		val ^= 0x80;

	if (_lcd_capable & INVERT_SS)
		val ^= 0x40;

	if (_lcd_capable & INVERT_RGB)
		val ^= 0x08;

	if (_lcd_ID == 0x65)//HX8352B
	{
		switch (rotation)
		{
		case 0:                    //PORTRAIT:
			val = 0x08;             //MY=0, MX=1, MV=0, ML=0, BGR=1
			break;

		case 1:                    //LANDSCAPE: 90 degrees
			val = 0x68;             //MY=0, MX=0, MV=1, ML=0, BGR=1
			break;

		case 2:                    //PORTRAIT_REV: 180 degrees
			val = 0xC8;             //MY=1, MX=0, MV=0, ML=1, BGR=1
			break;

		case 3:                    //LANDSCAPE_REV: 270 degrees
			val = 0xa8;             //MY=1, MX=1, MV=1, ML=1, BGR=1
			break;
		}

		_MW = 0x22;

		if (flag_write_bmp)
			WriteCmdData(0x16, val);
		else
			WriteCmdData(0x16, 0x08);

		_lcd_madctl = 0x08;
	}

	if ((rotation & 1) && ((_lcd_capable & MV_AXIS) == 0))
	{
		uint16_t x;
		x = _MC, _MC = _MP, _MP = x;
		x = _SC, _SC = _SP, _SP = x;    //.kbv check 0139
		x = _EC, _EC = _EP, _EP = x;    //.kbv check 0139
	}

	setAddrWindow(0, 0, width() - 1, height() - 1);
	vertScroll(0, HEIGHT, 0);   //reset scrolling after a rotation
}

void TFTScreen::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if (x < 0 || y < 0 || x >= width() || y >= height())
		return;

	if (_lcd_ID == 0x65)
	{
		if (!flag_write_bmp)
		{
			int16_t t;

			switch (rotation)
			{
			case 1:
				t = x;
				x = WIDTH - 1 - y;
				y = t;
				break;

			case 2:
				x = WIDTH - 1 - x;
				y = HEIGHT - 1 - y;
				break;

			case 3:
				t = x;
				x = y;
				y = HEIGHT - 1 - t;
				break;
			}
		}

		WriteCmdData(0x80, x >> 8);
		WriteCmdData(0x81, x);
		WriteCmdData(0x82, y >> 8);
		WriteCmdData(0x83, y);
	}

	WriteCmdData(_MW, color);
}

void TFTScreen::setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1)
{
	if (!flag_write_bmp)
	{
		int x0, y0, t;

		switch (rotation)
		{
		default:
			x0 = x;
			y0 = y;
			break;

		case 1:
			t = y;
			y = x;
			x = WIDTH - 1 - y1;
			y1 = x1;
			x1 = WIDTH - 1 - t;
			x0 = x1;
			y0 = y;
			break;

		case 2:
			t = x;
			x = WIDTH - 1 - x1;
			x1 = WIDTH - 1 - t;
			t = y;
			y = HEIGHT - 1 - y1;
			y1 = HEIGHT - 1 - t;
			x0 = x1;
			y0 = y1;
			break;

		case 3:
			t = x;
			x = y;
			y = HEIGHT - 1 - x1;
			x1 = y1;
			y1 = HEIGHT - 1 - t;
			x0 = x;
			y0 = y1;
			break;
	}
}

	WriteCmdData(0x02, x >> 8);
	WriteCmdData(0x03, x);
	WriteCmdData(0x04, x1 >> 8);
	WriteCmdData(0x05, x1);
	WriteCmdData(0x06, y >> 8);
	WriteCmdData(0x07, y);
	WriteCmdData(0x08, y1 >> 8);
	WriteCmdData(0x09, y1);
	WriteCmdData(0x80, x >> 8);
	WriteCmdData(0x81, x);
	WriteCmdData(0x82, y >> 8);
	WriteCmdData(0x83, y);
}

void TFTScreen::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	int16_t end;

	if (w < 0)
	{
		w = -w;
		x -= w;
	}                           //+ve w

	end = x + w;

	if (x < 0)
		x = 0;

	if (end > width())
		end = width();
	w = end - x;

	if (h < 0)
	{
		h = -h;
		y -= h;
	}                           //+ve h

	end = y + h;

	if (y < 0)
		y = 0;

	if (end > height())
		end = height();

	h = end - y;

	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_ACTIVE;
	WriteCmd(_MW);

	if (h > w)
	{
		end = h;
		h = w;
		w = end;
	}

	uint8_t hi = color >> 8, lo = color & 0xFF;
	CD_DATA;

	while (h-- > 0)
	{
		end = w;

		do
		{
			write8(hi);
			write8(lo);
		} while (--end != 0);
	}

	CS_IDLE;

	if (!(_lcd_capable & MIPI_DCS_REV1))
		setAddrWindow(0, 0, width() - 1, height() - 1);
}

void TFTScreen::pushColors(uint16_t * block, int16_t n, bool first)
{
	uint16_t color;
	CS_ACTIVE;
	if (first)
	{
		WriteCmd(_MW);
	}
	CD_DATA;
	while (n-- > 0)
	{
		color = *block++;
#ifdef ILI9327_SPECIAL
		color = ~color;
#endif
		write16(color);
	}
	CS_IDLE;
}

void TFTScreen::pushColors(uint8_t * block, int16_t n, bool first)
{
	uint16_t color;
	uint8_t h, l;
	CS_ACTIVE;
	if (first)
	{
		WriteCmd(_MW);
	}
	CD_DATA;
	while (n-- > 0)
	{
		h = (*block++);
		l = (*block++);
		color = h << 8 | l;
		write16(color);
	}
	CS_IDLE;
}

void TFTScreen::pushColors(const uint8_t * block, int16_t n, bool first)
{
	uint16_t color;
	uint8_t h, l;
	CS_ACTIVE;
	if (first)
	{
		WriteCmd(_MW);
	}

	CD_DATA;
	while (n-- > 0)
	{
		l = pgm_read_byte(block++);
		h = pgm_read_byte(block++);
		color = h << 8 | l;
		write16(color);
	}
	CS_IDLE;
}

void TFTScreen::vertScroll(int16_t top, int16_t scrollines, int16_t offset)
{
	int16_t bfa = HEIGHT - top - scrollines;
	int16_t vsp;
	int16_t sea = top;

	if (offset <= -scrollines || offset >= scrollines)
		offset = 0;

	vsp = top + offset;

	if (offset < 0)
		vsp += scrollines;

	sea = top + scrollines - 1;

	WriteCmdData(0x61, (1 << 1) | _lcd_rev);
	WriteCmdData(0x6A, vsp);
}

void TFTScreen::invertDisplay(boolean i)
{
	uint8_t val;
	_lcd_rev = ((_lcd_capable & REV_SCREEN) != 0) ^ i;
	WriteCmdData(0x61, _lcd_rev);
}


static void init_table(const void *table, int16_t size)
{
	uint8_t *p = (uint8_t *)table, dat[16];
	while (size > 0)
	{
		uint8_t cmd = pgm_read_byte(p++);
		uint8_t len = pgm_read_byte(p++);

		if (cmd == TFTLCD_DELAY8)
		{
			delay(len);
			len = 0;
		}
		else
		{
			for (uint8_t i = 0; i < len; i++)
			{
				dat[i] = pgm_read_byte(p++);
			}

			WriteCmdParamN(cmd, len, dat);
		}
		size -= len + 2;
	}
}

void TFTScreen::begin(uint16_t ID)
{
	int16_t *p16;               //so we can "write" to a const protected variable.
	reset();
	_lcd_xor = 0;

	switch (_lcd_ID = ID)
	{
	case 0x65:
		static const uint8_t HX8352B_regValues[] PROGMEM =
		{
			// Register setting for EQ setting
			0xe5, 1, 0x10,      //
			0xe7, 1, 0x10,      //
			0xe8, 1, 0x48,      //
			0xec, 1, 0x09,      //
			0xed, 1, 0x6c,      //
			// Power on Setting
			0x23, 1, 0x6F,      //
			0x24, 1, 0x57,      //
			0x25, 1, 0x71,      //
			0xE2, 1, 0x18,      //
			0x1B, 1, 0x15,      // 
			0x01, 1, 0x00,      //
			0x1C, 1, 0x03,      //
			// Power on sequence   
			0x19, 1, 0x01,      //
			TFTLCD_DELAY8, 5,
			0x1F, 1, 0x8C,      //
			0x1F, 1, 0x84,      //
			TFTLCD_DELAY8, 10,
			0x1F, 1, 0x94,      //
			TFTLCD_DELAY8, 10,
			0x1F, 1, 0xD4,      //
			TFTLCD_DELAY8, 5,
			// Gamma Setting 
			0x40, 1, 0x00,      //
			0x41, 1, 0x2B,      //
			0x42, 1, 0x29,      //
			0x43, 1, 0x3E,      //
			0x44, 1, 0x3D,      //
			0x45, 1, 0x3F,      //
			0x46, 1, 0x24,      //
			0x47, 1, 0x74,      //
			0x48, 1, 0x08,      //
			0x49, 1, 0x06,      //
			0x4A, 1, 0x07,      //
			0x4B, 1, 0x0D,      //
			0x4C, 1, 0x17,      //
			0x50, 1, 0x00,      //
			0x51, 1, 0x02,      //
			0x52, 1, 0x01,      //
			0x53, 1, 0x16,      //
			0x54, 1, 0x14,      //
			0x55, 1, 0x3F,      //
			0x56, 1, 0x0B,      //
			0x57, 1, 0x5B,      //
			0x58, 1, 0x08,      //
			0x59, 1, 0x12,      //
			0x5A, 1, 0x18,      //
			0x5B, 1, 0x19,      //
			0x5C, 1, 0x17,      //
			0x5D, 1, 0xFF,      //

			0x16, 1, 0x08,      //
			0x28, 1, 0x20,      //
			TFTLCD_DELAY8, 40,
			0x28, 1, 0x38,      //
			TFTLCD_DELAY8, 40,
			0x28, 1, 0x3C,      //

			0x02, 1, 0x00,      //
			0x03, 1, 0x00,      //
			0x04, 1, 0x00,      //
			0x05, 1, 0xef,      //
			0x06, 1, 0x00,      //
			0x07, 1, 0x00,      //
			0x08, 1, 0x01,      //
			0x09, 1, 0x8f,      //

			0x80, 1, 0x00,      //
			0x81, 1, 0x00,      //
			0x82, 1, 0x00,      //
			0x83, 1, 0x00,      //
			0x17, 1, 0x05,      //
		};

		init_table(HX8352B_regValues, sizeof(HX8352B_regValues));
		p16 = (int16_t *)& HEIGHT;
		*p16 = 400;
		break;
	}

	_lcd_rev = ((_lcd_capable & REV_SCREEN) != 0);
	setRotation(0);             //PORTRAIT
	invertDisplay(false);
}

//Button
TFTButton::TFTButton(void)
{
	_screen = 0;
}

void TFTButton::InitButton(TFTScreen *screen, int xPos, byte yPos, int xSize, byte ySize, uint16_t borderColor, uint16_t fillColor, uint16_t textColor, char* label)
{
	_screen = screen;
	_xPos = xPos;
	_yPos = yPos;
	_xSize = xSize;
	_ySize = ySize;
	_borderColor = borderColor;
	_fillColor = fillColor;
	_textColor = textColor;
	_label = label;
}

void TFTButton::DrawButton()
{
	_screen->fillRect(_xPos, _yPos, _xSize, _ySize, _fillColor);
	_screen->drawRect(_xPos, _yPos, _xSize, _ySize, _borderColor);
	_screen->setTextSize(1);

	int txPos = (_xPos + (_xSize / 2)) - (((strlen(_label) + 1) * 10) / 2);
	byte tyPos = _yPos + (_ySize / 2) + 8;

	_screen->setTextColor(_textColor);
	_screen->setCursor(txPos, tyPos);
	_screen->print(_label);
}

bool TFTButton::CheckButton(int xTouch, byte yTouch)
{
	prevPressed = pressed;

	if ((xTouch > _xPos) && (xTouch < _xPos + _xSize) &&
		(yTouch > _yPos) && (yTouch < _yPos + _ySize))
	{
		pressed = true;
		return true;
	}
	else
	{
		pressed = false;
		return false;
	}
}

void TFTButton::ChangeLabel(char* label)
{
	_label = label;
}