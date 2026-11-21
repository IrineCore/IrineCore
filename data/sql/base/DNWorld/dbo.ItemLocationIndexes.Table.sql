USE [dnworld]
GO
/****** Object:  Table [dbo].[ItemLocationIndexes]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ItemLocationIndexes](
	[OwnerCharacterID] [bigint] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemLocationIndex] [tinyint] NOT NULL,
 CONSTRAINT [PK_ItemLocationIndexes] PRIMARY KEY CLUSTERED 
(
	[OwnerCharacterID] ASC,
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
