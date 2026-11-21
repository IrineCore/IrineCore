USE [dnstaging]
GO
/****** Object:  Table [dbo].[DarkLairRank]    Script Date: 11/14/2026 9:00:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairRank](
	[WorldID] [int] NOT NULL,
	[MapID] [int] NOT NULL,
	[PartySize] [tinyint] NOT NULL,
	[PartyName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PlayRound] [smallint] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[CharacterID1] [bigint] NOT NULL,
	[CharacterLevel1] [tinyint] NOT NULL,
	[JobCode1] [tinyint] NOT NULL,
	[CharacterID2] [bigint] NULL,
	[CharacterLevel2] [tinyint] NULL,
	[JobCode2] [tinyint] NULL,
	[CharacterID3] [bigint] NULL,
	[CharacterLevel3] [tinyint] NULL,
	[JobCode3] [tinyint] NULL,
	[CharacterID4] [bigint] NULL,
	[CharacterLevel4] [tinyint] NULL,
	[JobCode4] [tinyint] NULL,
	[LogDate] [datetime] NOT NULL,
	[WorldRank] [int] NOT NULL,
	[TotalRank] [int] NOT NULL,
	[DarkLairRankID] [bigint] IDENTITY(1,1) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_DarkLairRank_WorldID_MapID_PartySize_CharacterID1_CharacterID2_CharacterID3_CharacterID4]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_DarkLairRank_WorldID_MapID_PartySize_CharacterID1_CharacterID2_CharacterID3_CharacterID4] ON [dbo].[DarkLairRank]
(
	[WorldID] ASC,
	[MapID] ASC,
	[PartySize] ASC,
	[CharacterID1] ASC,
	[CharacterID2] ASC,
	[CharacterID3] ASC,
	[CharacterID4] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
