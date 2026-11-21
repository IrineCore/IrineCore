USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyPVPLadderKillDeathByJob]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyPVPLadderKillDeathByJob](
	[WorldID] [int] NOT NULL,
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[KOWin] [int] NOT NULL,
	[KOLose] [int] NOT NULL,
 CONSTRAINT [PK_DailyPVPLadderKillDeathByJob] PRIMARY KEY CLUSTERED 
(
	[WorldID] ASC,
	[BasisDate] ASC,
	[CharacterID] ASC,
	[JobCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
