USE [dnworld]
GO
/****** Object:  Table [dbo].[NestClear]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestClear](
	[CharacterID] [bigint] NOT NULL,
	[MapID] [int] NOT NULL,
	[ClearCount] [tinyint] NOT NULL,
	[LastClearDate] [date] NOT NULL,
	[PCRoomClearCount] [tinyint] NOT NULL,
 CONSTRAINT [PK_NestClear] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MapID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[NestClear] ADD  CONSTRAINT [DF_NestClear_PCRoomClearCount]  DEFAULT ((0)) FOR [PCRoomClearCount]
GO
