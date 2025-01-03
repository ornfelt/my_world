﻿using System;
using System.Collections.Generic;
using System.Text;
using Client.Chat;
using Client.Chat.Definitions;
using Client.World.Definitions;

namespace Client.World.Network
{
    public partial class WorldSocket
    {
        public Race Race { get; private set; }
        public Class Class { get; private set; }

        //SMSG_GROUP_LIST = 125
        /*[PacketHandler(WorldCommand.SMSG_GROUP_LIST)]
        protected void HandleGroupList(InPacket packet)
        {
            GroupType groupType = (GroupType)packet.ReadByte();
            packet.ReadByte();
            packet.ReadByte();
            packet.ReadByte();
            if (groupType.HasFlag(GroupType.GROUPTYPE_LFG)) { }

            uint membersCount = packet.ReadUInt32();

            byte[] dump = packet.ReadToEnd();

            #region crap
            //ChatMessage message = new ChatMessage();
            //ChatChannel channel = new ChatChannel();
            //channel.Type = ChatMessageType.System;

            // index[0] 2 == 
            // index[0] 3 == 
            // index[0] 5 == 
            // 

            //StringBuilder Player = new StringBuilder();

            //byte[] dump = packet.ReadToEnd();


            //message.Message = Player.ToString();
            //message.Language = 0;
            //message.ChatTag = 0;
            //message.Sender = channel;
            //Game.UI.PresentChatMessage(message);
            #endregion
        }*/

        //SMSG_GROUP_INVITE = 111
        [PacketHandler(WorldCommand.SMSG_GROUP_INVITE)]
        protected void HandleGroupInvite(InPacket packet)
        {
            var message = new ChatMessage();
            var channel = new ChatChannel
            {
                Type = ChatMessageType.partyGroupInvitation
            };
            // index[0] 1 == invited to party group

            var builder = new StringBuilder();
            var dump = packet.ReadToEnd();

            var slen = ((int)packet.BaseStream.Length);
            for (var i = 1; i < slen; i++)
            {
                if ((char)dump[i] != 0x00)
                {
                    builder.Append((char)dump[i]);
                }
                else
                {
                    i = slen;
                }
            }

            channel.ChannelName = builder.ToString();
            message.Message = "";
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            Game.UI.PresentChatMessage(message);

            //new OutPacket(WorldCommand.CMSG_GROUP_ACCEPT, 4)
        }

        //CMSG_CHANNEL_INVITE = 163
        [PacketHandler(WorldCommand.CMSG_CHANNEL_INVITE)]
        protected void HandleChannelInvite(InPacket packet)
        {
            // 1 == party invite
            var dump = packet.ReadToEnd();
            //var tes = "";
            //new OutPacket(WorldCommand.CMSG_GROUP_ACCEPT, 4);
        }

        //SMSG_GROUP_DECLINE = 116
        [PacketHandler(WorldCommand.SMSG_GROUP_DECLINE)]
        protected void handleGroupDecline(InPacket packet)
        {
            var message = new ChatMessage();
            var channel = new ChatChannel
            {
                Type = ChatMessageType.System
            };

            var Player = new StringBuilder();

            var dump = packet.ReadToEnd();

            var slen = ((int)packet.BaseStream.Length);
            for (var i = 0; i < slen; i++)
            {
                if ((char)dump[i] != 0x00)
                {
                    Player.Append((char)dump[i]);
                }
                else
                {
                    i = slen;
                }
            }
            message.Message = Player + " declines your group invitation.";
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            //System.Threading.Thread.Sleep(100); // don't process too fast
            Game.UI.PresentChatMessage(message);
        }

        //SMSG_PARTY_COMMAND_RESULT = 127
        [PacketHandler(WorldCommand.SMSG_PARTY_COMMAND_RESULT)]
        protected void HandlePartyCommand(InPacket packet)
        {
            //var notifymsg = "";
            var message = new ChatMessage();
            var channel = new ChatChannel
            {
                Type = ChatMessageType.Party
            };

            // 1 byte after invited username 1 means user doesn't exist
            // 1 byte after invited username 5 means user already in group party

            var Player = new StringBuilder();
            var ResultCommand = new StringBuilder();
            var dump = packet.ReadToEnd();

            //packet.BaseStream.Position = 0;
            var state = 0;
            var slen = ((int)packet.BaseStream.Length);
            for (var i = 4; i < slen; i++)
            {
                if ((char)dump[i] != 0x00)
                {
                    if (state == 0)
                    {
                        Player.Append((char)dump[i]);
                    }
                    if (state == 1)
                    {
                        ResultCommand.Append((int)dump[i]);
                    }
                }
                else
                {
                    state += 1;
                    if (state == 2)
                    {
                        i = slen;
                    }
                }
            }

            //channel.ChannelName = "";

            switch (ResultCommand.ToString())
            {
                case "":
                    message.Message = "You invited '" + Player + "' to join your group.";
                    break;
                case "1":
                    message.Message = "Player '" + Player + "' doesn't exist.";
                    break;
                case "5":
                    message.Message = "Player '" + Player + "' is already in a group.";
                    break;
                case "7":
                    message.Message = "You are already in this group.";
                    break;
                default:
                    message.Message = "Unknown command in SMSG_PARTY_COMMAND_RESULT handler.";
                    break;
            }

            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            //System.Threading.Thread.Sleep(100); // don't process too fast
            Game.UI.PresentChatMessage(message);

        }

        /* 
        //SMSG_CHANNEL_LIST = 155,
        [PacketHandler(WorldCommand.SMSG_CHANNEL_NOTIFY)]
        protected void HandleChannelList(InPacket packet)
        {
            var notifymsg = "";
            ChatMessage message = new ChatMessage();
            ChatChannel channel = new ChatChannel();
            channel.Type = ChatMessageType.Channel;

            // index[0] 2 == joined
            // index[0] 3 == leaved
            // index[0] 5 == you are not in this group anymore
            // read channel name from index[1] till index[i] == 0x00

            StringBuilder builder = new StringBuilder();
            byte[] dump = packet.ReadToEnd();

            if ((char)dump[0] == 0x00) { notifymsg = "A user joined the channel."; }
            if ((char)dump[0] == 0x01) { notifymsg = "A user left the channel."; }
            if ((char)dump[0] == 0x02) { notifymsg = "Channel joined."; }
            if ((char)dump[0] == 0x03) { notifymsg = "Channel left."; }
            if ((char)dump[0] == 0x05) { notifymsg = "You already left this channel.."; }
            if ((char)dump[0] == 0x08) { notifymsg = "You are ower of the channel now."; } // 8
            if ((char)dump[0] == 0x0C) { notifymsg = "Moderation privileges given to you."; } // 12
            if ((char)dump[0] == 0x18) { notifymsg = ""; } // 24 Invited to join channel

            //packet.BaseStream.Position = 0;
            int slen = ((int)packet.BaseStream.Length);
            for (int i = 1; i < slen; i++)
            {
                if ((char)dump[i] != 0x00)
                {
                    builder.Append((char)dump[i]);
                }
                else
                {
                    i = slen;
                }
            }

            if ((char)dump[0] == 0x18)
            {
                channel.Type = ChatMessageType.ChannelInvitation;
            }



            channel.ChannelName = builder.ToString();
            message.Message = notifymsg; // + builder.ToString();
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            System.Threading.Thread.Sleep(100); // don't process too fast for automatic join
            Game.UI.PresentChatMessage(message);
        }*/

        [PacketHandler(WorldCommand.SMSG_MOTD)]
        protected void HandleServerMOTD(InPacket packet)
        {
            var message = new ChatMessage();
            var channel = new ChatChannel
            {
                Type = 0
            };

            var builder = new StringBuilder();
            packet.BaseStream.Position = 0;
            var slen = ((int)packet.BaseStream.Length);
            for (var i = 0; i < slen; i++)
            {
                var currentbyte = packet.BaseStream.ReadByte();
                if (currentbyte != 0x00 && currentbyte != 0x01)
                {
                    builder.Append((char)currentbyte);
                }
            }
            #region old crap
            /*result  "\u0001\0\0\0Welcome to x! Enjoy your stay!\0"   string
            result = result.Substring(4);
            result = result.TrimEnd('\0'); fuck trim and substring!*/
            #endregion
            message.Message = builder.ToString();
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            Game.UI.PresentChatMessage(message);
        }

        /// <summary>
        /// SMSG_GUILD_ROSTER = 138 GET GUILD player list data
        /// Glues together > CMSG_GUILD_MOTD = 145, SMSG_GUILD_EVENT = 146
        /// <summary>
        [PacketHandler(WorldCommand.SMSG_GUILD_ROSTER)]
        protected void HandleGuildRoster(InPacket packet)
        {

            Game.World.guildPlayer.Clear();
            Game.World.guildStatus.Clear();
            Game.World.guildClass.Clear();
            Game.World.guildLevel.Clear();
            Game.World.guildZone.Clear();
            Game.World.guildNote.Clear();
            Game.World.guildOfficerNote.Clear();

            var totalMembers = packet.ReadUInt32();
            var guildMOTD = packet.ReadCString();
            var guildInfo = packet.ReadCString();
            var numberOfRanks = packet.ReadUInt32();
            for (var i = 0; i < numberOfRanks; i++)
            {
                var rankRights = packet.ReadUInt32();
                var goldWithdrawlLimit = packet.ReadUInt32();
                for (var j = 0; j < 6; j++)
                {
                    var guildBankRights = packet.ReadUInt32();
                    var guildBankSlots = packet.ReadUInt32();
                }
            }
            for(var c = 0; c < totalMembers; c++)
            {
                var playerGuid = packet.ReadUInt64();
                int memberFlags = packet.ReadByte();
                var playerName = packet.ReadCString();
                var playerRank = packet.ReadUInt32();
                var playerLevel = packet.ReadByte();
                var playerClass = (Class)packet.ReadByte();
                var whyTheFuckHasntAnyEmulationProjectDocumentedWhatThisUInt8IsForFucksSake = packet.ReadByte();
                var playerZone = packet.ReadUInt32();
                float timeSinceLastLogIn = 0;
                if (memberFlags == 0)
                    timeSinceLastLogIn = packet.ReadSingle();
                var publicNote = packet.ReadCString();
                var officeNote = packet.ReadCString();
                if (memberFlags == 0)
                    continue;

                var status = "Online";

                if ((memberFlags & 8) != 0)
                    status = "Chat Client";

                else if ((memberFlags & 2) != 0)
                    status = "AFK";

                else if ((memberFlags & 4) != 0)
                    status = "DND";

                Game.World.guildPlayer.Add(playerName);
                Game.World.guildStatus.Add(status);
                Game.World.guildLevel.Add(playerLevel);
                Game.World.guildClass.Add(playerClass.ToString());
                Game.World.guildZone.Add(Extensions.GetZoneName((int)playerZone));
                Game.World.guildNote.Add(publicNote);
                Game.World.guildOfficerNote.Add(officeNote);
            }
            Game.UI.UpdateRoster("1");
            if (Game.World.sawGuildMessageBefore) return;
            var message = new ChatMessage();
            var channel = new ChatChannel {Type = 0};
            message.Message = guildMOTD;
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            Game.UI.PresentChatMessage(message);
            var message2 = new ChatMessage();
            var channel2 = new ChatChannel {Type = 0};
            message2.Message = guildInfo;
            message2.Language = 0;
            message2.ChatTag = 0;
            message2.Sender = channel;
            Game.UI.PresentChatMessage(message2);
            Game.World.sawGuildMessageBefore = true;
        }

        /*
        /// <summary>
        /// CMSG_WHO = 98 full who player list data
        /// Warning, this code burns your eyes !!! REFACTORING NEEDED !!
        /// At first I wrote code that handles data, check if in guild or not and read bytes and jump jump jump..
        /// but this code had a flaw/bug so I did this, really bad but whatever..
        /// 
        /// # extra debug information; #
        /// Layout who list ;
        /// 
        /// byte[0] = people found / online
        /// byte[1,2,3] skip == 0x0
        /// byte[4] = people found / online
        /// byte[5,6,7] skip == 0x0
        /// byte[8] = start first username ascii byte
        /// 
        /// keep in mind lvl is in hex and you need cast it back to dec.
        /// 
        /// Not in GUILD
        /// Username 00 00 lvl 00 00 00 Class 00 00 00 Race <Delimiter here>
        ///
        /// In GUILD
        /// Username 00 Guild 00 lvl 00 00 00 Class 00 00 00 Race <Delimiter here>
        ///
        /// Data delimiter 8x bytes 0x00 or 0x??, examples below:
        /// 00 00 00 00 01 00 00 00  after first user
        /// 00 00 00 00 D2 00 00 00  after second user
        /// 00 00 00 00 9B 0D 00 00  after third user
        /// 00 00 00 01 9B 0D 00 00  and so on..
        /// 00 00 00 00 D2 00 00 00
        /// 00 00 00 01 2B 11 00 00
        /// 00 00 00 00 EF 05 00 00
        /// </summary>
        [PacketHandler(WorldCommand.SMSG_WHO)]
        protected void HandleWhoList(InPacket packet)
        {
            ChatMessage message = new ChatMessage();
            ChatChannel channel = new ChatChannel();
            channel.Type = 0;

            StringBuilder newbuild = new StringBuilder();

            byte[] dump = packet.ReadToEnd();
            for (int i = 0; i < dump.Length; i++)
            {
                if((int)dump[i] >= 32 && (int)dump[i] <= 122)
                {
                    
                    if((int)dump[i] == 32)
                    {
                        newbuild.Append("_");
                    } else
                    {
                        newbuild.Append((char)dump[i]);
                    }
                    
                }
                if((char)dump[i] == 0)
                {
                    newbuild.Append("  ");
                }
                if ((byte)dump[i] >= 1 && (byte)dump[i] <= 64)
                {
                    if((byte)dump[i] != 32)
                    {
                        newbuild.Append((int)dump[i]);
                    }
                }
            }

            StringBuilder builder = new StringBuilder();

            string players = "";
            string[] AllInfos = newbuild.ToString().Split(new[] { "  " }, StringSplitOptions.None); // '\x0020'

            players = players + (int)dump[0] + " People found online:\r\n";

            for(int i = 0; i < AllInfos.Length; i++)
            {
                string current = AllInfos[i];
                if(Regex.Matches(current, @"[a-zA-Z]").Count > 2)
                {
                    players = players + "[] " + AllInfos[i].ToString();
                    if(AllInfos[i+1].ToString() != "")
                    {
                        players = players + " Guild: " + AllInfos[i + 1].ToString().Replace("_", " ");
                        string Level = AllInfos[i + 2].ToString();
                        if(Regex.Matches(Level, @"[a-zA-Z]").Count > 0)
                        {
                            int lvl = (int)System.Convert.ToChar(AllInfos[i + 2].ToString());
                            players = players + " LvL: " + lvl;

                        }
                        else
                        {
                            Level = Regex.Replace(Level, @"[^\d]", "");
                            players = players + " LvL: " + Level.ToString();
                        }
                        Class = (Class)(int)System.Convert.ToDecimal(AllInfos[i + 5].ToString());
                        Race = (Race)(int)System.Convert.ToDecimal(AllInfos[i + 8].ToString());
                        players = players + " Class: " + Class.ToString();
                        players = players + " Race: " + Race.ToString() + "\r\n";
                        i += 3;
                    }
                    else
                    {
                        players = players + " Guild: none";
                        string Level = AllInfos[i + 2].ToString();
                        if (Regex.Matches(Level, @"[a-zA-Z]").Count > 0)
                        {
                            int lvl = (int)System.Convert.ToChar(AllInfos[i + 2].ToString());
                            players = players + " LvL: " + lvl;

                        }
                        else
                        {
                            Level = Regex.Replace(Level, @"[^\d]", "");
                            players = players + " LvL: " + Level.ToString();
                        }
                        Class = (Class)(int)System.Convert.ToDecimal(AllInfos[i + 5].ToString());
                        Race = (Race)(int)System.Convert.ToDecimal(AllInfos[i + 8].ToString());
                        players = players + " Class: " + Class.ToString();
                        players = players + " Race: " + Race.ToString() + "\r\n";
                    }
                }
            }

            message.Message = players.ToString();
            message.Language = 0;
            message.ChatTag = 0;
            message.Sender = channel;
            Game.UI.PresentChatMessage(message);
        }*/
        [PacketHandler(WorldCommand.SMSG_MESSAGECHAT)]
        [PacketHandler(WorldCommand.SMSG_GM_MESSAGECHAT)]
        protected void HandleMessageChat(InPacket packet)
        {
            var chatType = (ChatMessageType)packet.ReadByte();
            var language = (Language)packet.ReadInt32();
            var senderGuid = packet.ReadUInt64();
            var unkInt = packet.ReadUInt32();

            UInt32 senderNameLen = 0;
            string senderName = null;
            UInt64 receiverGuid = 0;
            UInt32 receiverNameLen = 0;
            string receiverName = null;
            string channelName = null;

            switch (chatType)
            {
                case ChatMessageType.MonsterSay:
                case ChatMessageType.MonsterParty:
                case ChatMessageType.MonsterYell:
                case ChatMessageType.MonsterWhisper:
                case ChatMessageType.MonsterEmote:
                case ChatMessageType.RaidBossEmote:
                case ChatMessageType.RaidBossWhisper:
                case ChatMessageType.BattleNet:
                    senderNameLen = packet.ReadUInt32();
                    senderName = packet.ReadCString();
                    receiverGuid = packet.ReadUInt64();
                    if (receiverGuid != 0 && !receiverGuid.IsPlayer() && !receiverGuid.IsPet())
                    {
                        receiverNameLen = packet.ReadUInt32();
                        receiverName = packet.ReadCString();
                    }
                    break;
                case ChatMessageType.WhisperForeign:
                    senderNameLen = packet.ReadUInt32();
                    senderName = packet.ReadCString();
                    receiverGuid = packet.ReadUInt64();
                    break;
                case ChatMessageType.BattlegroundNeutral:
                case ChatMessageType.BattlegroundAlliance:
                case ChatMessageType.BattlegroundHorde:
                    receiverGuid = packet.ReadUInt64();
                    if (receiverGuid != 0 && !receiverGuid.IsPlayer())
                    {
                        receiverNameLen = packet.ReadUInt32();
                        receiverName = packet.ReadCString();
                    }
                    break;
                case ChatMessageType.Achievement:
                case ChatMessageType.GuildAchievement:
                    receiverGuid = packet.ReadUInt64();
                    break;
                default:
                    if (packet.Header.Command == WorldCommand.SMSG_GM_MESSAGECHAT)
                    {
                        senderNameLen = packet.ReadUInt32();
                        senderName = packet.ReadCString();
                    }

                    if (chatType == ChatMessageType.Channel)
                    {
                        channelName = packet.ReadCString();
                    }
                    
                    receiverGuid = packet.ReadUInt64();
                    break;
            }

            var messageLen = packet.ReadUInt32();
            var message = packet.ReadCString();
            var chatTag = (ChatTag)packet.ReadByte();

            if (chatType == ChatMessageType.Achievement || chatType == ChatMessageType.GuildAchievement)
            {
                var achievementId = (int)packet.ReadUInt32();
                
                var achName = Extensions.GetAchName(achievementId);
                message = message.Replace("$a", achName);
                message = message.Replace("%s", "$REPLACEME53582");

            }

            var channel = new ChatChannel {Type = chatType};

            if (chatType == ChatMessageType.Channel)
                channel.ChannelName = channelName;

            var chatMessage = new ChatMessage
            {
                Message = message,
                Language = language,
                ChatTag = chatTag,
                Sender = channel
            };
            if (chatMessage.Language.ToString() == "Addon") return;
            //! If we know the name of the sender GUID, use it
            //! For system messages sender GUID is 0, don't need to do anything fancy
            if (senderGuid == 0 || !string.IsNullOrEmpty(senderName)
                                || Game.World.PlayerNameLookup.TryGetValue(senderGuid, out senderName))
            {
                chatMessage.Sender.Sender = senderName;
                chatMessage.Message = chatMessage.Message.Replace("$REPLACEME53582", senderName);
                Game.UI.PresentChatMessage(chatMessage);
                return;
            }

            //! If not we place the message in the queue,
            //! .. either existent
            if (Game.World.QueuedChatMessages.TryGetValue(senderGuid, out var messageQueue))
                messageQueue.Enqueue(chatMessage);
            //! or non existent
            else
            {
                messageQueue = new Queue<ChatMessage>();
                messageQueue.Enqueue(chatMessage);
                Game.World.QueuedChatMessages.Add(senderGuid, messageQueue);
            }

            //! Furthermore we send CMSG_NAME_QUERY to the server to retrieve the name of the sender
            var response = new OutPacket(WorldCommand.CMSG_NAME_QUERY);
            response.Write(senderGuid);
            Game.SendPacket(response);
            //! Enqueued chat will be printed when we receive SMSG_NAME_QUERY_RESPONSE
        }



        [PacketHandler(WorldCommand.SMSG_NOTIFICATION)]
        protected void HandleMessageChat5(InPacket packet)
        {
            var notification = packet.ReadCString();
            var channel = new ChatChannel {Type = ChatMessageType.System};
            var chatMessage = new ChatMessage
            {
                Message = notification,
                Language = Language.Universal,
                ChatTag = 0,
                Sender = channel
            };
            Game.UI.PresentChatMessage(chatMessage);
        }

        [PacketHandler(WorldCommand.SMSG_SERVER_MESSAGE)]
        protected void HandleMessageChat6(InPacket packet)
        {
            var type = packet.ReadUInt32();
            /*
            SERVER_MSG_SHUTDOWN_TIME      = 1,
            SERVER_MSG_RESTART_TIME       = 2,
            SERVER_MSG_STRING             = 3,
            SERVER_MSG_SHUTDOWN_CANCELLED = 4,
            SERVER_MSG_RESTART_CANCELLED  = 5
            */
            if (type < 3) return;
            var notification = packet.ReadCString();
            var channel = new ChatChannel {Type = ChatMessageType.System};
            var chatMessage = new ChatMessage
            {
                Message = notification,
                Language = Language.Universal,
                ChatTag = 0,
                Sender = channel
            };
            Game.UI.PresentChatMessage(chatMessage);
        }

        [PacketHandler(WorldCommand.CMSG_MESSAGECHAT)]
        protected void HandleMessageChat2(InPacket packet)
        {
            var type = (ChatMessageType)packet.ReadByte();
            var lang = (Language)packet.ReadInt32();
            var guid = packet.ReadUInt64();
            var unkInt = packet.ReadInt32();

            var channel = new ChatChannel {Type = type};

            if (type == ChatMessageType.Channel)
                channel.ChannelName = packet.ReadCString();

            var sender = packet.ReadUInt64();

            var message = new ChatMessage();
            var textLen = packet.ReadInt32();
            message.Message = packet.ReadCString();
            message.Language = lang;
            message.ChatTag = (ChatTag)packet.ReadByte();
            message.Sender = channel;

            Game.UI.PresentChatMessage(message);
        }

        //  CMSG_NAME_QUERY = 80,
        //  SMSG_NAME_QUERY_RESPONSE = 81,
        [PacketHandler(WorldCommand.SMSG_NAME_QUERY_RESPONSE)]
        protected void HandleSmsgNameQueryResponse(InPacket packet)
        {
            var message = new ChatMessage();
            var channel = new ChatChannel {Type = 0};
            var result = Encoding.UTF8.GetString(packet.ReadToEnd());
        }

        [PacketHandler(WorldCommand.SMSG_CHAT_PLAYER_NOT_FOUND)]
        protected void HandleChatPlayerNotFound(InPacket packet)
        {
            var channelgc = new ChatChannel {Type = 0};
            var message = new ChatMessage
            {
                Message = $"Player '{packet.ReadCString()}' doesn't exist!",
                Language = 0,
                ChatTag = 0,
                Sender = channelgc
            };
            Game.UI.PresentChatMessage(message);
        }

        [PacketHandler(WorldCommand.SMSG_PONG)]
        protected void HandlePong(InPacket packet)
        {
            var ping = packet.ReadUInt32();
        }

    }
}
