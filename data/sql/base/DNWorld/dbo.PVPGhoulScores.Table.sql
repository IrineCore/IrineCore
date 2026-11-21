USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPGhoulScores]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPGhoulScores](
	[CharacterID] [bigint] NOT NULL,
	[PlayCount] [int] NOT NULL,
	[GhoulWin] [int] NOT NULL,
	[HumanWin] [int] NOT NULL,
	[TimeOver] [int] NOT NULL,
	[GhoulKill] [int] NOT NULL,
	[HumanKill] [int] NOT NULL,
	[HolyWaterUse] [int] NOT NULL,
	[UpdateDate] [datetime] NOT NULL,
 CONSTRAINT [PK_PVPGhoulScores] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
