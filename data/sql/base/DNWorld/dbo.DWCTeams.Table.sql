USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCTeams]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCTeams](
	[DWCTeamID] [int] IDENTITY(1,1) NOT NULL,
	[DWCTeamName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CloseFlag] [bit] NOT NULL,
	[CloseDate] [datetime2](0) NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
	[DWCPoint] [int] NOT NULL,
	[CreateDate] [datetime2](0) NOT NULL,
	[TodayWin] [smallint] NOT NULL,
	[TodayLose] [smallint] NOT NULL,
	[TodayDraw] [smallint] NOT NULL,
	[ConsecutiveWin] [smallint] NOT NULL,
	[ConsecutiveLose] [smallint] NOT NULL,
	[WeeklyPlayCount] [smallint] NOT NULL,
	[LastPlayDate] [datetime2](0) NOT NULL,
	[DWCHiddenPoint] [int] NOT NULL,
 CONSTRAINT [PK_DWCTeams] PRIMARY KEY CLUSTERED 
(
	[DWCTeamID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UNF_DWCTeams_DWCTeamName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_DWCTeams_DWCTeamName] ON [dbo].[DWCTeams]
(
	[DWCTeamName] ASC
)
WHERE ([CloseFlag]=CONVERT([bit],(0),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
