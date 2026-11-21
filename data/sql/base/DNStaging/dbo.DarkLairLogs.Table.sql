USE [dnstaging]
GO
/****** Object:  Table [dbo].[DarkLairLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairLogs](
	[LogDate] [datetime] NOT NULL,
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
	[JobCode4] [tinyint] NULL
) ON [PRIMARY]
GO
