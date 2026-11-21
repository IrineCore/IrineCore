USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyPVPLadderScoresByJob]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyPVPLadderScoresByJob](
	[WorldID] [int] NOT NULL,
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
 CONSTRAINT [PK_DailyPVPLadderScoresByJob] PRIMARY KEY CLUSTERED 
(
	[WorldID] ASC,
	[BasisDate] ASC,
	[CharacterID] ASC,
	[JobCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
