namespace Tests;

internal static class TestUtils
{
    public static byte[] StringToByteArrayNoPadding(string hex) => StringToByteArray(hex, false);

    public static byte[] StringToByteArray(string hex, bool padding = true)
    {
        if (hex.Length % 2 == 1)
        {
            throw new Exception("The binary key cannot have an odd number of digits");
        }

        var length = hex.Length >> 1;
        var offset = 0;

        if (length % 2 != 0 && padding)
        {
            length += 1;
            offset = 1;
        }

        var arr = new byte[length];

        for (var i = 0; i < hex.Length >> 1; ++i)
        {
            arr[i + offset] = (byte)((GetHexVal(hex[i << 1]) << 4) + GetHexVal(hex[(i << 1) + 1]));
        }

        Array.Reverse(arr);

        return arr;

        int GetHexVal(char v)
        {
            var val = (int)v;
            //For uppercase A-F letters:
            //return val - (val < 58 ? 48 : 55);
            //For lowercase a-f letters:
            //return val - (val < 58 ? 48 : 87);
            //Or the two combined, but a bit slower:
            return val - (val < 58 ? 48 : val < 97 ? 55 : 87);
        }
    }
}