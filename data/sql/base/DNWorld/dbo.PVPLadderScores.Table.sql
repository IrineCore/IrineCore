USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderScores]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderScores](
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
 CONSTRAINT [PK_PVPLadderScores] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[PVPLadderCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
