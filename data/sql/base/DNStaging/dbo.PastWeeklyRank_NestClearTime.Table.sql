USE [dnstaging]
GO
/****** Object:  Table [dbo].[PastWeeklyRank_NestClearTime]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastWeeklyRank_NestClearTime](
	[BasisDate] [date] NOT NULL,
	[StageID] [int] NOT NULL,
	[ClearTime] [int] NOT NULL,
	[CharacterID1] [bigint] NULL,
	[CharacterLevel1] [tinyint] NULL,
	[JobCode1] [tinyint] NULL,
	[CharacterID2] [bigint] NULL,
	[CharacterLevel2] [tinyint] NULL,
	[JobCode2] [tinyint] NULL,
	[CharacterID3] [bigint] NULL,
	[CharacterLevel3] [tinyint] NULL,
	[JobCode3] [tinyint] NULL,
	[CharacterID4] [bigint] NULL,
	[CharacterLevel4] [tinyint] NULL,
	[JobCode4] [tinyint] NULL,
	[CharacterID5] [bigint] NULL,
	[CharacterLevel5] [tinyint] NULL,
	[JobCode5] [tinyint] NULL,
	[CharacterID6] [bigint] NULL,
	[CharacterLevel6] [tinyint] NULL,
	[JobCode6] [tinyint] NULL,
	[CharacterID7] [bigint] NULL,
	[CharacterLevel7] [tinyint] NULL,
	[JobCode7] [tinyint] NULL,
	[CharacterID8] [bigint] NULL,
	[CharacterLevel8] [tinyint] NULL,
	[JobCode8] [tinyint] NULL,
	[TotalRanking] [int] NOT NULL,
 CONSTRAINT [PK_PastWeeklyRank_NestClearTime] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[StageID] ASC,
	[TotalRanking] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
