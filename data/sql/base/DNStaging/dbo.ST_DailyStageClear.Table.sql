USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_DailyStageClear]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_DailyStageClear](
	[BasisDate] [date] NOT NULL,
	[StageID] [int] NOT NULL,
	[DifficultyCode] [tinyint] NOT NULL,
	[ChallengeCount] [int] NOT NULL,
	[ClearCount] [int] NOT NULL,
	[AvgClearTime] [int] NOT NULL,
	[MinClearTime] [int] NOT NULL,
 CONSTRAINT [PK_ST_DailyStageClear] PRIMARY KEY CLUSTERED 
(
	[BasisDate] ASC,
	[StageID] ASC,
	[DifficultyCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
