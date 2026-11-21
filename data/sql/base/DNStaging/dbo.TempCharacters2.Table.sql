USE [dnstaging]
GO
/****** Object:  Table [dbo].[TempCharacters2]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TempCharacters2](
	[CharacterID] [bigint] NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterIndex] [tinyint] NOT NULL,
	[VillageFirstVisitFlag] [bit] NOT NULL,
	[SkillResetFlag] [bit] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterExp] [int] NOT NULL,
	[MissionScore] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[Coin] [bigint] NOT NULL,
	[WarehouseCoin] [bigint] NOT NULL,
	[SkillPoint] [smallint] NOT NULL,
	[LastLoginDate] [datetime2](0) NOT NULL,
	[AccountLevelCode] [tinyint] NULL,
	[PCRoomRebirthCoin] [tinyint] NULL,
	[RebirthCoin] [tinyint] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UC_TempCharacters2_CharacterID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_TempCharacters2_CharacterID] ON [dbo].[TempCharacters2]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
