USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyPVPLadder2]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyPVPLadder2](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[Ladder0] [int] NULL,
	[Ladder9] [int] NULL,
	[Ladder10] [int] NULL,
	[Ladder11] [int] NULL,
	[LadderPoint1800] [int] NULL,
	[PVPLadderGradePoint_AVG] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_ST_DailyPVPLadder2_BasisDate_WorldID]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_ST_DailyPVPLadder2_BasisDate_WorldID] ON [dbo].[ST_DailyPVPLadder2]
(
	[BasisDate] ASC,
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
