USE [dnworld]
GO
/****** Object:  Table [dbo].[HiddenCharacters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[HiddenCharacters](
	[CharacterID] [bigint] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_HiddenCharacters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[HiddenCharacters] ADD  CONSTRAINT [DF_HiddenCharacters_RegisterDate]  DEFAULT (getdate()) FOR [RegisterDate]
GO
