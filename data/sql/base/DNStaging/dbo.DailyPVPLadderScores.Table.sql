USE [dnstaging]
GO
/****** Object:  Table [dbo].[DailyPVPLadderScores]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyPVPLadderScores](
	[WorldID] [int] NOT NULL,
	[BasisDate] [date] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PVPLadderCode] [tinyint] NOT NULL,
	[PVPLadderGradePoint] [int] NOT NULL,
	[HiddenPVPLadderGradePoint] [int] NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
	[TodayWin] [smallint] NOT NULL,
	[TodayLose] [smallint] NOT NULL,
	[TodayDraw] [smallint] NOT NULL,
	[ConsecutiveWin] [smallint] NOT NULL,
	[ConsecutiveLose] [smallint] NOT NULL,
	[LastPVPLadderScoreDate] [datetime2](0) NOT NULL,
	[WeeklyPVPCount] [smallint] NOT NULL,
 CONSTRAINT [PK_DailyPVPLadderScores] PRIMARY KEY CLUSTERED 
(
	[WorldID] ASC,
	[BasisDate] ASC,
	[CharacterID] ASC,
	[PVPLadderCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
