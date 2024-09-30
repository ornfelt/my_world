using System;
using System.Security.Cryptography;
using WowSrp.Internal;

namespace WowSrp
{
    /// <summary>
    ///     Verifies the integrity of local files by hashing them.
    ///     This is sent by the client in
    ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>.
    ///     The server can verify it if it wants, it's not required by the client.
    /// </summary>
    public static class Integrity
    {
        /// <summary>
        ///     Amount of bytes returned by <see cref="RandomIntegritySalt" /> and required by the salts for
        ///     the various checks.
        /// </summary>
        public const int IntegritySaltSize = 16;

        private static readonly byte[] Zero = new byte[Constants.ProofLength];

        /// <summary>
        ///     Convenience function to return random salt.
        /// </summary>
        public static byte[] RandomIntegritySalt() => Utils.RandomizedArray(IntegritySaltSize);

        /// <summary>
        ///     Check where are files are concatenated into a single array.
        /// </summary>
        /// <param name="allFiles">All relevant files.</param>
        /// <param name="checksumSalt">
        ///     Sent by the server in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
        ///     .
        /// </param>
        /// <param name="clientPublicKey">
        ///     Sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>.
        /// </param>
        public static byte[] GenericCheck(byte[] allFiles, byte[] checksumSalt,
            ReadOnlySpan<byte> clientPublicKey)
        {
            var checksum = new HMACSHA1(checksumSalt).ComputeHash(allFiles);
            var full = Utils.ConcatArrays(clientPublicKey, checksum.AsSpan());
            return Utils.Sha1(full);
        }

        /// <summary>
        ///     Specific check for Windows version.
        ///     This is the same as <see cref="GenericCheck" /> but with specific arguments for the individual files.
        /// </summary>
        /// <param name="wowExe">WoW.exe</param>
        /// <param name="fmodDll">fmod.dll</param>
        /// <param name="ijl15Dll">ijl15.dll</param>
        /// <param name="dbghelpDll">dbghelp.dll</param>
        /// <param name="unicowsDll">unicows.dll</param>
        /// <param name="checksumSalt">
        ///     Sent by the server in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
        ///     .
        /// </param>
        /// <param name="clientPublicKey">
        ///     Sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>.
        /// </param>
        public static byte[] WindowsCheck(ReadOnlySpan<byte> wowExe,
            ReadOnlySpan<byte> fmodDll,
            ReadOnlySpan<byte> ijl15Dll,
            ReadOnlySpan<byte> dbghelpDll,
            ReadOnlySpan<byte> unicowsDll,
            byte[] checksumSalt,
            ReadOnlySpan<byte> clientPublicKey)
        {
            var allFiles = Utils.ConcatArrays(wowExe, fmodDll, ijl15Dll, dbghelpDll, unicowsDll);
            return GenericCheck(allFiles, checksumSalt, clientPublicKey);
        }

        /// <summary>
        ///     Specific check for Mac version.
        ///     This is the same as <see cref="GenericCheck" /> but with specific arguments for the individual files.
        /// </summary>
        /// <param name="worldOfWarcraft">MacOS/WorldOfWarcraft</param>
        /// <param name="infoPlist">Info.plist</param>
        /// <param name="objectsXib">Resources/Main.nib/objects.xib</param>
        /// <param name="wowIcns">Resources/wow.icns</param>
        /// <param name="pkgInfo">PkgInfo</param>
        /// <param name="checksumSalt">
        ///     Sent by the server in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
        ///     .
        /// </param>
        /// <param name="clientPublicKey">
        ///     Sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>.
        /// </param>
        public static byte[] MacCheck(ReadOnlySpan<byte> worldOfWarcraft,
            ReadOnlySpan<byte> infoPlist,
            ReadOnlySpan<byte> objectsXib,
            ReadOnlySpan<byte> wowIcns,
            ReadOnlySpan<byte> pkgInfo,
            byte[] checksumSalt,
            ReadOnlySpan<byte> clientPublicKey)
        {
            var allFiles = Utils.ConcatArrays(worldOfWarcraft, infoPlist, objectsXib, wowIcns, pkgInfo);
            return GenericCheck(allFiles, checksumSalt, clientPublicKey);
        }

        /// <summary>
        ///     Check for all clients during reconnect.
        /// </summary>
        /// <param name="checksumSalt">
        ///     Sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_reconnect_proof_client.html">CMD_AUTH_RECONNECT_PROOF_Client</a>
        ///     .
        /// </param>
        public static byte[] ReconnectCheck(ReadOnlySpan<byte> checksumSalt)
        {
            var full = Utils.ConcatArrays(checksumSalt, Zero);
            return Utils.Sha1(full);
        }
    }
}