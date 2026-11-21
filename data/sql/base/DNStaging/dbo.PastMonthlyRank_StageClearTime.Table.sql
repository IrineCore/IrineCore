USE [dnstaging]
GO
/****** Object:  Table [dbo].[PastMonthlyRank_StageClearTime]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PastMonthlyRank_StageClearTime](
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[StageID] [int] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[ClearTime] [int] NOT NULL,
	[ClearDate] [datetime2](0) NOT NULL,
	[TotalRank] [int] NOT NULL,
	[WorldRank] [int] NOT NULL,
	[WorldCharacterClassRank] [int] NOT NULL,
	[CharacterClassRank] [int] NULL,
 CONSTRAINT [PK_PastMonthlyRank_StageClearTime] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[CharacterID] ASC,
	[StageID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [OldData]
) ON [OldData]
GO
