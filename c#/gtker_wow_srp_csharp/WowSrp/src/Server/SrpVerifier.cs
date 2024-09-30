using WowSrp.Internal;

namespace WowSrp.Server
{
    /// <summary>
    ///     First step of server authentication.
    ///     Can be used to generate database values.
    /// </summary>
    public readonly struct SrpVerifier
    {
        /// <summary>
        ///     Constructor when generating database values.
        ///     This should only be used for creating new users, not when actually authenticating.
        /// </summary>
        public SrpVerifier(string username, string password)
        {
            Username = Utils.ConvertUsernameOrPassword(username, nameof(username));
            password = Utils.ConvertUsernameOrPassword(password, nameof(password));

            Salt = Utils.RandomizedArray(Constants.KeyLength);
            PasswordVerifier = Implementation.CalculatePasswordVerifier(username, password, Salt);
        }

        /// <summary>
        ///     Constructor for values from the database.
        ///     This should be the preferred constructor.
        /// </summary>
        public SrpVerifier(string username, byte[] passwordVerifier, byte[] salt)
        {
            Username = Utils.ConvertUsernameOrPassword(username, nameof(username));

            Utils.AssertArrayLength(passwordVerifier, Constants.KeyLength, nameof(passwordVerifier));
            PasswordVerifier = passwordVerifier;

            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));
            Salt = salt;
        }

        /// <summary>
        ///     Username of the user.
        /// </summary>
        public string Username { get; }

        /// <summary>
        ///     Password verifier of the user.
        /// </summary>
        public byte[] PasswordVerifier { get; }

        /// <summary>
        ///     Salt of the user.
        ///     This must be sent to the client.
        /// </summary>
        public byte[] Salt { get; }

        /// <summary>
        ///     Converts the database values into a proof.
        /// </summary>
        public SrpProof IntoProof() => new SrpProof(Username, PasswordVerifier, Salt);
    }
}