using System;

namespace WowSrp.Internal
{
    internal class Arc4
    {
        private const int StateSize = 0x100;
        private readonly byte[] _state = new byte[StateSize];
        private byte _i;
        private byte _j;

        public Arc4(byte[] key, int drop)
        {
            // https://en.wikipedia.org/wiki/RC4#Key-scheduling_algorithm_(KSA)
            for (var i = 0; i < StateSize; i++)
            {
                _state[i] = (byte)i;
            }

            var j = 0;
            for (var i = 0; i < StateSize; i++)
            {
                j = (byte)((j + key[i % key.Length] + _state[i]) & 255);

                (_state[i], _state[j]) = (_state[j], _state[i]);
            }

            if (drop != 0)
            {
                var dropArray = new byte[drop];
                ApplyKeyStream(dropArray);
            }
        }

        public void ApplyKeyStream(Span<byte> data)
        {
            // https://en.wikipedia.org/wiki/RC4#Pseudo-random_generation_algorithm_(PRGA)
            for (var i = 0; i < data.Length; i++)
            {
                _i = (byte)((_i + 1) % StateSize);
                _j = (byte)((_j + _state[_i]) % StateSize);

                (_state[_i], _state[_j]) = (_state[_j], _state[_i]);

                data[i] = (byte)(_state[(_state[_i] + _state[_j]) % StateSize] ^ data[i]);
            }
        }
    }
}