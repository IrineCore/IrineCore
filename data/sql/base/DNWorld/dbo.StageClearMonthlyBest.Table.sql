USE [dnworld]
GO
/****** Object:  Table [dbo].[StageClearMonthlyBest]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageClearMonthlyBest](
	[MapID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ClearGradeCode] [tinyint] NOT NULL,
	[ClearPoint] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_StageClearMonthlyBest] PRIMARY KEY CLUSTERED 
(
	[MapID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
