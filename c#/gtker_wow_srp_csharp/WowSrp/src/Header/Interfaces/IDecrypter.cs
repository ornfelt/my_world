using System;

namespace WowSrp.Header
{
    /// <summary>
    ///     General decrypter.
    /// </summary>
    public interface IDecrypter
    {
        /// <summary>
        ///     Decrypt arbitrary data.
        ///     Do not use this directly, prefer the helper methods in <see cref="IServerDecrypter" /> or
        ///     <see cref="IClientDecrypter" /> instead.
        /// </summary>
        public void Decrypt(Span<byte> data);
    }
}