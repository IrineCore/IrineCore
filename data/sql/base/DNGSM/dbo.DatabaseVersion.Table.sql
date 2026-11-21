USE [dngsm]
GO
/****** Object:  Table [dbo].[DatabaseVersion]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DatabaseVersion](
	[version] [int] NOT NULL,
	[registerDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DatabaseVersion] PRIMARY KEY CLUSTERED 
(
	[version] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
ALTER TABLE [dbo].[DatabaseVersion] ADD  CONSTRAINT [DF_DatabaseVersion]  DEFAULT (getdate()) FOR [registerDate]
GO
