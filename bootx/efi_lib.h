#pragma once
#include "efi.h"
#include <stdbool.h>
#include <stdarg.h>

static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* cerr;
static EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* cout;
static EFI_BOOT_SERVICES* bs;

void Initilize(EFI_SYSTEM_TABLE* systemtable)
{
    cout = systemtable->ConOut;
    cerr = systemtable->StdErr;
    bs = systemtable->BootServices;

    //cout->ClearScreen(cout);
}

typedef struct {
    EFI_BLOCK_IO_PROTOCOL* biop;
    UINTN CurrentPositionLBA;

    UINTN ReadCount;
}DISK_FILE;

DISK_FILE* CreateDiskFile(EFI_BLOCK_IO_PROTOCOL* biop)
{
    DISK_FILE* file = NULL;
    if (EFI_ERROR(bs->AllocatePool(EfiLoaderData,sizeof(DISK_FILE),(VOID**)&file)))
    {
        return NULL;
    }

    file->biop = biop;
    file->CurrentPositionLBA = 0;

    return file;
}

void DestroyDiskFile(DISK_FILE* file)
{
    bs->FreePool(file);

    file = NULL;
}

void fseek(DISK_FILE* file,UINTN offsetLBA)
{
    file->CurrentPositionLBA = offsetLBA;
}

EFI_STATUS fread(void* buffer,UINTN size,DISK_FILE* file)
{
    EFI_STATUS s = file->biop->ReadBlocks(file->biop,file->biop->Media->MediaId,file->CurrentPositionLBA,size,buffer);
    file->ReadCount++;
    return s;
}


UINTN strlen_c16(CHAR16 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

UINTN strlen(CHAR8 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

CHAR16 *strrev_c16(CHAR16 *s) {
    if (!s) return s;

    CHAR16 *start = s, *end = s + strlen_c16(s)-1;
    while (start < end) {
        CHAR16 temp = *end;  // Swap
        *end-- = *start;
        *start++ = temp;
    }

    return s;
}
BOOLEAN isdigit_c16(CHAR16 c) {
    return c >= u'0' && c <= u'9';
}

CHAR16 *strcpy_c16(CHAR16 *dst, CHAR16 *src) {
    if (!dst || !src) return dst;

    CHAR16 *result = dst;
    while (*src) *dst++ = *src++;

    *dst = u'\0';   // Null terminate

    return result;
}



CHAR16 *strcat_c16(CHAR16 *dst, CHAR16 *src) {
    CHAR16 *s = dst;

    while (*s) s++;             // Go until null terminator

    while (*src) *s++ = *src++; // Copy src to dst at null position

    *s = u'\0';                 // Null terminate new string

    return dst; 
}

void CHAR8toCHAR16(const CHAR8* src, CHAR16* dst) {
    while (*src) {
        *dst = (CHAR16)(*src); // zero-extend ASCII to 16 bits
        src++;
        dst++;
    }
    *dst = 0; // null-terminate CHAR16 string
}

BOOLEAN
add_int_to_buf_c16(UINTN number, UINT8 base, BOOLEAN signed_num, UINTN min_digits, CHAR16 *buf, 
                   UINTN *buf_idx) {
    const CHAR16 *digits = u"0123456789ABCDEF";
    CHAR16 buffer[24];  // Hopefully enough for UINTN_MAX (UINT64_MAX) + sign character
    UINTN i = 0;
    BOOLEAN negative = FALSE;

    if (base > 16) {
        cerr->OutputString(cerr, u"Invalid base specified!\r\n");
        return FALSE;    // Invalid base
    }

    // Only use and print negative numbers if decimal and signed True
    if (base == 10 && signed_num && (INTN)number < 0) {
       number = -(INTN)number;  // Get absolute value of correct signed value to get digits to print
       negative = TRUE;
    }

    do {
       buffer[i++] = digits[number % base];
       number /= base;
    } while (number > 0);

    while (i < min_digits) buffer[i++] = u'0'; // Pad with 0s

    // Add negative sign for decimal numbers
    if (base == 10 && negative) buffer[i++] = u'-';

    // NULL terminate string
    buffer[i--] = u'\0';

    // Reverse buffer to read left to right
    strrev_c16(buffer);

    // Add number string to input buffer for printing
    for (CHAR16 *p = buffer; *p; p++) {
        buf[*buf_idx] = *p;
        *buf_idx += 1;
    }
    return TRUE;
}

bool format_string_c16(CHAR16 *buf, CHAR16 *fmt, va_list args) {
    bool result = true;
    CHAR16 charstr[2] = {0};    
    UINTN buf_idx = 0;

    for (UINTN i = 0; fmt[i] != u'\0'; i++) {
        if (fmt[i] == u'%') {
            bool alternate_form = false;
            UINTN min_field_width = 0;
            UINTN precision = 0;
            UINTN length_bits = 0;  
            UINTN num_printed = 0;      // # of digits/chars printed for numbers or strings
            UINT8 base = 0;
            bool input_precision = false;
            bool signed_num   = false;
            bool int_num      = false;
            bool double_num   = false;
            bool left_justify = false;  // Left justify text from '-' flag instead of default right justify
            bool space_flag   = false;
            bool plus_flag    = false;
            CHAR16 padding_char = ' ';  // '0' or ' ' depending on flags
            i++;

            // Check for flags
            while (true) {
                switch (fmt[i]) {
                    case u'#':
                        // Alternate form
                        alternate_form = true;
                        i++;
                        continue;

                    case u'0':
                        // 0-pad numbers on the left, unless '-' or precision is also defined
                        padding_char = '0'; 
                        i++;
                        continue;

                    case u' ':
                        // Print a space before positive signed number conversion or empty string
                        //   number conversions
                        space_flag = true;
                        if (plus_flag) space_flag = false;  // Plus flag '+' overrides space flag
                        i++;
                        continue;

                    case u'+':
                        // Always print +/- before a signed number conversion
                        plus_flag = true;
                        if (space_flag) space_flag = false; // Plus flag '+' overrides space flag
                        i++;
                        continue;

                    case u'-':
                        left_justify = true;
                        i++;
                        continue;

                    default:
                        break;
                }
                break; // No more flags
            }

            // Check for minimum field width e.g. in "8.2" this would be 8
            if (fmt[i] == u'*') {
                // Get int argument for min field width
                min_field_width = va_arg(args, int);
                i++;
            } else {
                // Get number literal from format string
                while (isdigit_c16(fmt[i])) 
                    min_field_width = (min_field_width * 10) + (fmt[i++] - u'0');
            }

            // Check for precision/maximum field width e.g. in "8.2" this would be 2
            if (fmt[i] == u'.') {
                input_precision = true; 
                i++;
                if (fmt[i] == u'*') {
                    // Get int argument for precision
                    precision = va_arg(args, int);
                    i++;
                } else {
                    // Get number literal from format string
                    while (isdigit_c16(fmt[i])) 
                        precision = (precision * 10) + (fmt[i++] - u'0');
                }
            }

            // Check for Length modifiers e.g. h/hh/l/ll
            if (fmt[i] == u'h') {
                i++;
                length_bits = 16;       // h
                if (fmt[i] == u'h') {
                    i++;
                    length_bits = 8;    // hh
                }
            } else if (fmt[i] == u'l') {
                i++;
                length_bits = 32;       // l
                if (fmt[i] == u'l') {
                    i++;
                    length_bits = 64;    // ll
                }
            }

            // Check for conversion specifier
            switch (fmt[i]) {
                case u'c': {
                    // Print CHAR16 value; printf("%c", char)
                    if (length_bits == 8)
                        charstr[0] = (char)va_arg(args, int);   // %hhc "ascii" or other 8 bit char
                    else
                        charstr[0] = (CHAR16)va_arg(args, int); // Assuming 16 bit char16_t

                    // Only add non-null characters, to not end string early
                    if (charstr[0]) buf[buf_idx++] = charstr[0];    
                }
                break;

                case u's': {
                    // Print CHAR16 string; printf("%s", string)
                    if (length_bits == 8) {
                        char *string = va_arg(args, char*);         // %hhs; Assuming 8 bit ascii chars
                        while (*string) {
                            buf[buf_idx++] = *string++;
                            if (++num_printed == precision) break;  // Stop printing at max characters
                        }

                    } else {
                        CHAR16 *string = va_arg(args, CHAR16*);     // Assuming 16 bit char16_t
                        while (*string) {
                            buf[buf_idx++] = *string++;
                            if (++num_printed == precision) break;  // Stop printing at max characters
                        }
                    }
                }
                break;

                case u'd': {
                    // Print INT32; printf("%d", number_int32)
                    int_num = true;
                    base = 10;
                    signed_num = true;
                }
                break;

                case u'x': {
                    // Print hex UINTN; printf("%x", number_uintn)
                    int_num = true;
                    base = 16;
                    signed_num = false;
                    if (alternate_form) {
                        buf[buf_idx++] = u'0';
                        buf[buf_idx++] = u'x';
                    }
                }
                break;

                case u'u': {
                    // Print UINT32; printf("%u", number_uint32)
                    int_num = true;
                    base = 10;
                    signed_num = false;
                }
                break;

                case u'b': {
                    // Print UINTN as binary; printf("%b", number_uintn)
                    int_num = true;
                    base = 2;
                    signed_num = false;
                    if (alternate_form) {
                        buf[buf_idx++] = u'0';
                        buf[buf_idx++] = u'b';
                    }
                }
                break;

                case u'o': {
                    // Print UINTN as octal; printf("%o", number_uintn)
                    int_num = true;
                    base = 8;
                    signed_num = false;
                    if (alternate_form) {
                        buf[buf_idx++] = u'0';
                        buf[buf_idx++] = u'o';
                    }
                }
                break;

                case u'f': {
                    // Print INTN rounded float value
                    double_num = true;
                    signed_num = true;
                    base = 10;
                    if (!input_precision) precision = 6;    // Default decimal places to print
                }
                break;

                default:
                    strcpy_c16(buf, u"Invalid format specifier: %");
                    charstr[0] = fmt[i];
                    strcat_c16(buf, charstr);
                    strcat_c16(buf, u"\r\n");
                    result = false;
                    goto end;
                    break;
            }

            if (int_num) {
                // Number conversion: Integer
                UINT64 number = 0;
                switch (length_bits) {
                    case 0:
                    case 32: 
                    default:
                        // l
                        number = va_arg(args, UINT32);
                        if (signed_num) number = (INT32)number;
                        break;

                    case 8:
                        // hh
                        number = (UINT8)va_arg(args, int);
                        if (signed_num) number = (INT8)number;
                        break;

                    case 16:
                        // h
                        number = (UINT16)va_arg(args, int);
                        if (signed_num) number = (INT16)number;
                        break;

                    case 64:
                        // ll
                        number = va_arg(args, UINT64);
                        if (signed_num) number = (INT64)number;
                        break;
                }

                // Add space before positive number for ' ' flag
                if (space_flag && signed_num && (INTN)number >= 0) buf[buf_idx++] = u' ';    

                // Add sign +/- before signed number for '+' flag
                if (plus_flag && signed_num) buf[buf_idx++] = (INTN)number >= 0 ? u'+' : u'-';

                add_int_to_buf_c16(number, base, signed_num, precision, buf, &buf_idx);
            }

            if (double_num) {
                // Number conversion: Float/Double
                double number = va_arg(args, double);
                INTN whole_num = 0;

                // Get digits before decimal point
                whole_num = (INTN)number;
                if (whole_num < 0) whole_num = -whole_num;

                UINTN num_digits = 0;
                do {
                   num_digits++; 
                   whole_num /= 10;
                } while (whole_num > 0);

                // Add digits to write buffer
                add_int_to_buf_c16(number, base, signed_num, num_digits, buf, &buf_idx);

                // Print decimal digits equal to precision value, 
                //   if precision is explicitly 0 then do not print
                if (!input_precision || precision != 0) {
                    buf[buf_idx++] = u'.';      // Add decimal point

                    if (number < 0.0) number = -number; // Ensure number is positive
                    whole_num = (INTN)number;
                    number -= whole_num;                // Get only decimal digits
                    signed_num = FALSE;                 // Don't print negative sign for decimals

                    // Move precision # of decimal digits before decimal point 
                    //   using base 10, number = number * 10^precision
                    for (UINTN i = 0; i < precision; i++)
                        number *= 10;

                    // Add digits to write buffer
                    add_int_to_buf_c16(number, base, signed_num, precision, buf, &buf_idx);
                }
            }

            // Flags are defined such that 0 is overruled by left justify and precision
            if (padding_char == u'0' && (left_justify || precision > 0))
                padding_char = u' ';

            // Add padding depending on flags (0 or space) and left/right justify
            INTN diff = min_field_width - buf_idx;
            if (diff > 0) {
                if (left_justify) {
                    // Append padding to minimum width, always spaces
                    while (diff--) buf[buf_idx++] = u' ';   
                } else {
                    // Right justify
                    // Copy buffer to end of buffer
                    INTN dst = min_field_width-1, src = buf_idx-1;
                    while (src >= 0)  buf[dst--] = buf[src--];  // e.g. "TEST\0\0" -> "TETEST"

                    // Overwrite beginning of buffer with padding
                    dst = (int_num && alternate_form) ? 2 : 0;  // Skip 0x/0b/0o/... prefix
                    while (diff--) buf[dst++] = padding_char;   // e.g. "TETEST" -> "  TEST"
                }
            }

        } else {
            // Not formatted string, print next character
            buf[buf_idx++] = fmt[i];
        }
    }

    end:
    buf[buf_idx] = u'\0'; 
    va_end(args);
    return result;
}

bool vfprintf_c16(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stream, CHAR16 *fmt, va_list args) {
    CHAR16 buf[1024];
    if (!format_string_c16(buf, fmt, args)) 
        return false;
    
    return !EFI_ERROR(stream->OutputString(stream, buf));
}

bool printf_c16(CHAR16 *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return vfprintf_c16(cout, fmt, args);
}

VOID *memcpy(VOID *dst, VOID *src, UINTN len) {
    UINT8 *p = dst, *q = src;
    while (len--) *p++ = *q++;
    return dst;
}


INTN memcmp(VOID *m1, VOID *m2, UINTN len) {
    UINT8 *p = m1;
    UINT8 *q = m2;
    for (UINTN i = 0; i < len; i++)
        if (p[i] != q[i]) return (INTN)(p[i]) - (INTN)(q[i]);

    return 0;
}


CHAR16* GetEFIError(EFI_STATUS status)
{
    switch (status)
    {
        case EFI_SUCCESS: return L"EFI_SUCCESS";

        case EFI_LOAD_ERROR: return L"EFI_LOAD_ERROR";
        case EFI_INVALID_PARAMETER: return L"EFI_INVALID_PARAMETER";
        case EFI_UNSUPPORTED: return L"EFI_UNSUPPORTED";
        case EFI_BAD_BUFFER_SIZE: return L"EFI_BAD_BUFFER_SIZE";
        case EFI_BUFFER_TOO_SMALL: return L"EFI_BUFFER_TOO_SMALL";
        case EFI_NOT_READY: return L"EFI_NOT_READY";
        case EFI_DEVICE_ERROR: return L"EFI_DEVICE_ERROR";
        case EFI_WRITE_PROTECTED: return L"EFI_WRITE_PROTECTED";
        case EFI_OUT_OF_RESOURCES: return L"EFI_OUT_OF_RESOURCES";
        case EFI_VOLUME_CORRUPTED: return L"EFI_VOLUME_CORRUPTED";
        case EFI_VOLUME_FULL: return L"EFI_VOLUME_FULL";
        case EFI_NO_MEDIA: return L"EFI_NO_MEDIA";
        case EFI_MEDIA_CHANGED: return L"EFI_MEDIA_CHANGED";
        case EFI_NOT_FOUND: return L"EFI_NOT_FOUND";
        case EFI_ACCESS_DENIED: return L"EFI_ACCESS_DENIED";
        case EFI_NO_RESPONSE: return L"EFI_NO_RESPONSE";
        case EFI_NO_MAPPING: return L"EFI_NO_MAPPING";
        case EFI_TIMEOUT: return L"EFI_TIMEOUT";
        case EFI_NOT_STARTED: return L"EFI_NOT_STARTED";
        case EFI_ALREADY_STARTED: return L"EFI_ALREADY_STARTED";
        case EFI_ABORTED: return L"EFI_ABORTED";
        case EFI_ICMP_ERROR: return L"EFI_ICMP_ERROR";
        case EFI_TFTP_ERROR: return L"EFI_TFTP_ERROR";
        case EFI_PROTOCOL_ERROR: return L"EFI_PROTOCOL_ERROR";
        case EFI_INCOMPATIBLE_VERSION: return L"EFI_INCOMPATIBLE_VERSION";
        case EFI_SECURITY_VIOLATION: return L"EFI_SECURITY_VIOLATION";
        case EFI_CRC_ERROR: return L"EFI_CRC_ERROR";
        case EFI_END_OF_MEDIA: return L"EFI_END_OF_MEDIA";
        case EFI_END_OF_FILE: return L"EFI_END_OF_FILE";
        case EFI_INVALID_LANGUAGE: return L"EFI_INVALID_LANGUAGE";
        case EFI_COMPROMISED_DATA: return L"EFI_COMPROMISED_DATA";
        case EFI_IP_ADDRESS_CONFLICT: return L"EFI_IP_ADDRESS_CONFLICT";
        case EFI_HTTP_ERROR: return L"EFI_HTTP_ERROR";

        default: return L"UNKNOWN_EFI_STATUS";
    }
}