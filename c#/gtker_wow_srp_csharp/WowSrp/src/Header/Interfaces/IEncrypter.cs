using System;

namespace WowSrp.Header
{
    /// <summary>
    ///     General encrypter.
    /// </summary>
    public interface IEncrypter
    {
        /// <summary>
        ///     Encrypt arbitrary data.
        ///     Do not use this directly, prefer the helper methods in <see cref="IServerEncrypter" /> or
        ///     <see cref="IClientEncrypter" /> instead.
        /// </summary>
        public void Encrypt(Span<byte> data);
    }
}