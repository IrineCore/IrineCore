USE [dnworld]
GO
/****** Object:  Table [dbo].[PVPCharacterClassScores]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PVPCharacterClassScores](
	[CharacterID] [bigint] NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[Win] [int] NOT NULL,
	[Lose] [int] NOT NULL,
 CONSTRAINT [PK_PVPCharacterClassScores] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[CharacterClassCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
