USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPLadderScoresByJob]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPLadderScoresByJob](
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
	[Draw] [int] NOT NULL,
 CONSTRAINT [PK_PVPLadderScoresByJob] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[JobCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
