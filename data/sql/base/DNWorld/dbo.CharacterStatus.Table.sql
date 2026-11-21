USE [dnworld]
GO
/****** Object:  Table [dbo].[CharacterStatus]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[CharacterStatus](
	[CharacterID] [bigint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterExp] [int] NOT NULL,
	[MissionScore] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[HairColor] [int] NOT NULL,
	[EyeColor] [int] NOT NULL,
	[SkinColor] [int] NOT NULL,
	[LastMapID] [int] NOT NULL,
	[LastVillageMapID] [int] NOT NULL,
	[LastDarkLairVillageMapID] [int] NOT NULL,
	[LastVillageGateNumber] [tinyint] NOT NULL,
	[PositionX] [int] NOT NULL,
	[PositionY] [int] NOT NULL,
	[PositionZ] [int] NOT NULL,
	[Rotate] [float] NOT NULL,
	[Coin] [bigint] NOT NULL,
	[WarehouseCoin] [bigint] NOT NULL,
	[LastRebirthCoinDate] [smalldatetime] NOT NULL,
	[RebirthCoin] [tinyint] NOT NULL,
	[PCRoomRebirthCoin] [tinyint] NOT NULL,
	[SkillPoint] [smallint] NOT NULL,
	[LastFatigueDate] [smalldatetime] NOT NULL,
	[Fatigue] [smallint] NOT NULL,
	[PCRoomFatigue] [smallint] NOT NULL,
	[WeeklyFatigue] [smallint] NOT NULL,
	[ViewCashEquip1Flag] [bit] NOT NULL,
	[ViewCashEquip2Flag] [bit] NOT NULL,
	[CheckSumBin] [binary](4) NOT NULL,
	[LastLogoutDate] [datetime2](0) NOT NULL,
	[LastLoginDate] [datetime2](0) NOT NULL,
	[LastSessionID] [int] NOT NULL,
	[LastLoginIP] [int] NOT NULL,
	[LastMapEntranceDate] [datetime2](0) NOT NULL,
	[EventFatigue] [smallint] NOT NULL,
	[EventFatigueGainDate] [smalldatetime] NOT NULL,
	[VIPFatigue] [smallint] NOT NULL,
	[LastQuestCheckDate] [smalldatetime] NOT NULL,
	[PVPLadderPoint] [int] SPARSE  NULL,
	[LastServerType] [tinyint] NULL,
	[TimeEventDate] [datetime2](0) NULL,
	[SkillPoint2] [smallint] NOT NULL,
	[CurrentSkillCode] [tinyint] NOT NULL,
	[ViewCashEquipmentBitmap] [int] NOT NULL,
	[GuildMaxLevelRewardFlag] [bit] NULL,
	[GuildMaxLevelRewardDate] [datetime] NULL,
	[LastStampDate] [datetime2](0) NULL,
	[TalismanSlotBitmap] [int] SPARSE  NULL,
	[CheckSumFlag] [bit] NULL,
	[LikeCount] [int] NOT NULL,
 CONSTRAINT [PK_CharacterStatus] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING OFF
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastLogoutDate]  DEFAULT (getdate()) FOR [LastLogoutDate]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastLoginDate]  DEFAULT (getdate()) FOR [LastLoginDate]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastSessionID]  DEFAULT ((0)) FOR [LastSessionID]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastLoginIP]  DEFAULT ((0)) FOR [LastLoginIP]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastMapEntranceDate]  DEFAULT (getdate()) FOR [LastMapEntranceDate]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_EventFatigue]  DEFAULT ((0)) FOR [EventFatigue]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_EventFatigueGainDate]  DEFAULT (getdate()-(1)) FOR [EventFatigueGainDate]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_VIPFatigue]  DEFAULT ((0)) FOR [VIPFatigue]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LastQuestCheckDate]  DEFAULT (getdate()-(1)) FOR [LastQuestCheckDate]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_SkillPoint2]  DEFAULT ((0)) FOR [SkillPoint2]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_CurrentSkillCode]  DEFAULT ((1)) FOR [CurrentSkillCode]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_ViewCashEquipmentBitmap]  DEFAULT ((65535)) FOR [ViewCashEquipmentBitmap]
GO
ALTER TABLE [dbo].[CharacterStatus] ADD  CONSTRAINT [DF_CharacterStatus_LikeCount]  DEFAULT ((0)) FOR [LikeCount]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_CharacterExp] CHECK  (([CharacterExp]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_CharacterExp]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_CharacterLevel] CHECK  (([CharacterLevel]>=(1)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_CharacterLevel]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_Coin] CHECK  (([Coin]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_Coin]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_MissionScore] CHECK  (([MissionScore]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_MissionScore]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_PCRoomRebirthCoin] CHECK  (([PCRoomRebirthCoin]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_PCRoomRebirthCoin]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_RebirthCoin] CHECK  (([RebirthCoin]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_RebirthCoin]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_SkillPoint] CHECK  (([SkillPoint]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_SkillPoint]
GO
ALTER TABLE [dbo].[CharacterStatus]  WITH CHECK ADD  CONSTRAINT [CK_CharacterStatus_WarehouseCoin] CHECK  (([WarehouseCoin]>=(0)))
GO
ALTER TABLE [dbo].[CharacterStatus] CHECK CONSTRAINT [CK_CharacterStatus_WarehouseCoin]
GO
