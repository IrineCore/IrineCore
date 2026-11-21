USE [dnstaging]
GO
/****** Object:  Table [dbo].[Rank_MissionScore]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Rank_MissionScore](
	[CharacterID] [bigint] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[MissionScore] [int] NOT NULL,
	[TotalRank] [int] NOT NULL,
	[WorldRank] [int] NOT NULL,
	[WorldCharacterClassRank] [int] NOT NULL,
	[CharacterClassRank] [int] NULL,
 CONSTRAINT [PK_Rank_MissionScore] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
