USE [dnworld]
GO
/****** Object:  Table [dbo].[CompleteChallenges]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CompleteChallenges](
	[CharacterID] [bigint] NOT NULL,
	[ChallengeID] [tinyint] NOT NULL,
	[Weekday] [int] NOT NULL,
	[CompleteDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_CompleteChallenges] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[ChallengeID] ASC,
	[Weekday] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
