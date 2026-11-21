USE [dnworld]
GO
/****** Object:  Table [dbo].[QuickSlots]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[QuickSlots](
	[CharacterID] [bigint] NOT NULL,
	[QuickSlotIndex] [tinyint] NOT NULL,
	[ObjectTypeCode] [tinyint] NOT NULL,
	[ObjectValue] [bigint] NOT NULL,
 CONSTRAINT [PK_QuickSlots] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[QuickSlotIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
