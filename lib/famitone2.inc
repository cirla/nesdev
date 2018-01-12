;this CA65 file automatically reserves RAM for FamiTone2 variables and includes the code

.export _FamiToneInit, _FamiToneSfxInit, _FamiToneMusicPlay, _FamiToneSfxPlay, _FamiToneUpdate

.segment "CODE"
.include "famitone2.s"

.segment "ZEROPAGE"
FT_TEMP:		.res FT_TEMP_SIZE

.segment "BSS"
FT_BASE_ADR:	.res FT_BASE_SIZE

