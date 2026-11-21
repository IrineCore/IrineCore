USE [dnworld]
GO
/****** Object:  Table [dbo].[DailyLikes]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DailyLikes](
	[CharacterID] [bigint] NOT NULL,
	[LikeCharacterID] [bigint] NOT NULL,
	[LikeDate] [datetime2](7) NOT NULL,
 CONSTRAINT [PK_DailyLikes] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[LikeCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
